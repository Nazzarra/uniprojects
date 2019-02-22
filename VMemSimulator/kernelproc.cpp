#include "kernelproc.h"
#include "kernelsys.h"
#include "process.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <mutex>

//#define _VM_PRINT_PREF_TIME

#ifdef _VM_PRINT_PREF_TIME
#include <windows.h>
#endif

KernelProcess::KernelProcess(ProcessId pid)
	: pid(pid), pmt(nullptr)
{
}

KernelProcess::~KernelProcess()
{
	KernelSystem::pInstance->killProcess(this);
}

ProcessId KernelProcess::getPid() const {
	return pid;
}

Status KernelProcess::createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags)
{
	if (!isAllignedToPageStart(startAddress))
		return Status::TRAP;

	PageNum pageStart = getVAPage(startAddress);
	PageNum pageEnd = pageStart + segmentSize - 1;

	if (pageEnd >= PAGE_NO_OVERFLOW)
		return Status::TRAP;

	if (isPageRangeOverlappingSegments(pageStart, pageEnd))
		return Status::TRAP;
	
	unsigned short missingPMTs = getMissingPMTsInRange(pageStart, pageEnd);
	
	if (missingPMTs > KernelSystem::pInstance->getFreePmtBlocks())
		return Status::TRAP;
	
	allocatMissingPMTsInRange(pageStart, pageEnd);

	unsigned char read = getReadFlag(flags);
	unsigned char write = getWriteFlag(flags);
	unsigned char execute = getExecuteFlag(flags);

	doOpToPagesInInterval(pageStart, pageEnd, [&read, &write, &execute](PageDescriptor& pg) {
		pg.history = 0;
		pg.flags = 0;
		pg.flagsCoW = 0;
		pg.rootDescr = nullptr;
		pg.frame = nullptr;
		pg.cluster = ~(0LU);
		pg.setRead(read);
		pg.setWrite(write);
		pg.setExecute(execute);
		pg.setValid(1);
	});

	segments.emplace_back(pageStart, pageEnd, (AccessType)flags);

	return Status::OK;
}

Status KernelProcess::loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags, void* content)
{
	//Content must be a multiple of PageSize
	if (createSegment(startAddress, segmentSize, flags) == Status::TRAP)
		return Status::TRAP;

	PageNum startPage = getVAPage(startAddress);
	PageNum endPage = startPage + segmentSize - 1;
	for (PageNum i = startPage; i <= endPage; ++i)
	{
		PmtLevelTwo* pmtTwo = pmt->pPmtTable[getPagePMTOneEntry(i)];
		PageDescriptor& pg = pmtTwo->pageDescr[getPagePMTTwoEntry(i)];
		if (!pg.frame) //Check not necessary, create never gives frames initially
			KernelSystem::pInstance->pageFault(this, i << OFFSET_BITS);
	
		memcpy(pg.frame->frameStart, (char*)content + (i - startPage) * PAGE_SIZE, PAGE_SIZE);
	}

	return Status::OK;
}

Status KernelProcess::deleteSegment(VirtualAddress startAddress)
{
	if (!isAllignedToPageStart(startAddress))
		return Status::TRAP;

	PageNum startPage = getVAPage(startAddress);
	for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
		if (iter->pageStart == startPage) {
			PageNum pageNo = iter->pageStart;
			doOpToPagesInInterval(iter->pageStart, iter->pageEnd, [this, &pageNo](PageDescriptor& pg)
			{
				if (!pg.isCoW()) {
					if (pg.frame)
						KernelSystem::pInstance->framePool.freeFrame(pg.frame);
					if (pg.isValidCluster())
						KernelSystem::pInstance->diskService.freeCluster(pg.cluster);
				}

				else {
					if (pg.frame) {
						if (pg.frame->pageDescr == &pg) {
							KernelProcess* nextProc = nullptr;
							if (pg.isNextPidValid())
								nextProc = KernelSystem::pInstance->processes[pg.cowList.nextProcPid];
							else
								nextProc = KernelSystem::pInstance->processes[pg.cowList.prevProcPid];
							pg.frame->pageDescr = &nextProc->getPageDescriptor(pg.frame->pageNo);
						}
					}
					deletePageFromCoWList(pageNo);
				}
				pg.setValid(0);
				++pageNo;
			});

			unsigned short pmtOneStart = getPagePMTOneEntry(iter->pageStart);
			unsigned short pmtOneEnd = getPagePMTOneEntry(iter->pageEnd);

			for (unsigned short i = pmtOneStart + 1; i < pmtOneEnd; ++i)
				freeLevelTwoPMT(i);

			PmtLevelTwo* pPmtTwo = pmt->pPmtTable[pmtOneStart];
			if (!pPmtTwo->hasValidPages())
				freeLevelTwoPMT(pmtOneStart);

			pPmtTwo = pmt->pPmtTable[pmtOneEnd];
			if (pPmtTwo && !pPmtTwo->hasValidPages())
				freeLevelTwoPMT(pmtOneEnd);

			segments.erase(iter);
			return Status::OK;
		}
	}

	for (auto iter : sharedSegments) {
		if (iter.second.pKernelSegment->isCoW && iter.second.segment.pageStart == startPage) {
			disconnectSharedSegment(iter.second.pKernelSegment->name.c_str());
			return Status::OK;
		}
	}

	return Status::TRAP;
}

Status KernelProcess::createSharedSegment(VirtualAddress startAddress, PageNum segmentSize, const char * name, AccessRight flags, bool isCoWSegment)
{
	if (!isAllignedToPageStart(startAddress))
		return Status::TRAP;

	PageNum startPage = getVAPage(startAddress);
	PageNum endPage = startPage + segmentSize - 1;
	
	if (endPage >= PAGE_NO_OVERFLOW)
		return Status::TRAP;

	if (isPageRangeOverlappingSegments(startPage, endPage))
		return Status::TRAP;

	auto segIter = KernelSystem::pInstance->sharedSegments.find(name);
	if (segIter != KernelSystem::pInstance->sharedSegments.end()) {
		if (segIter->second->size != segmentSize)
			return Status::TRAP;

		auto& procs = segIter->second->processes;
		if (std::find(procs.cbegin(), procs.cend(), this) != procs.cend())
			return Status::TRAP; //Cannot connect to a named shared segment twice(why would you anyway?)

		if (segIter->second->isCoW && !isCoWSegment)
			return Status::TRAP; //The user has been naughty and has been trying to connect to a system created shared segment
	}

	unsigned short missingPMTs = getMissingPMTsInRange(startPage, endPage);
	if (missingPMTs > KernelSystem::pInstance->getFreePmtBlocks())
		return Status::TRAP;

	allocatMissingPMTsInRange(startPage, endPage);

	unsigned char read = getReadFlag(flags);
	unsigned char write = getWriteFlag(flags);
	unsigned char execute = getExecuteFlag(flags);
	SharedSegmentKernel* kernelSegment;
	if (segIter == KernelSystem::pInstance->sharedSegments.end()) {
		//Should never pass here if isCoWSegment = true
		kernelSegment = new SharedSegmentKernel(name, segmentSize, isCoWSegment);
		KernelSystem::pInstance->sharedSegments[name] = kernelSegment;
	}
	else
		kernelSegment = segIter->second;

	if(kernelSegment->processes.size() == 0){
		//Check if this segment has existed before
		if (kernelSegment->diskDump != nullptr) {
			PageNum pageIter = 0;
			SharedSegmentDiskDump** dump = kernelSegment->diskDump;

			doOpToPagesInInterval(startPage, endPage, [&read, &write, &execute, &pageIter, &dump](PageDescriptor& pg) {
				pg.history = 0;
				pg.flags = 0;
				pg.flagsCoW = 0;
				pg.rootDescr = &pg;
				pg.frame = nullptr;
				pg.cluster = dump[pageIter / SHARED_SEGMENT_CLUSTERS_PER_DUMP]->pageCluster[pageIter % SHARED_SEGMENT_CLUSTERS_PER_DUMP];
				pg.setRead(read);
				pg.setWrite(write);
				pg.setExecute(execute);
				pg.setValid(1);
				++pageIter;
			});

			unsigned dumpSize = kernelSegment->size / SHARED_SEGMENT_CLUSTERS_PER_DUMP;
			if (kernelSegment->size % SHARED_SEGMENT_CLUSTERS_PER_DUMP != 0)
				++dumpSize;

			for (unsigned i = 0; i < dumpSize; ++i)
				KernelSystem::pInstance->freePmt(dump[i]);

			delete[] dump;
			kernelSegment->diskDump = nullptr;
		}
		else {
			doOpToPagesInInterval(startPage, endPage, [&read, &write, &execute](PageDescriptor& pg) {
				pg.history = 0;
				pg.flags = 0;
				pg.flagsCoW = 0;
				pg.rootDescr = &pg;
				pg.frame = nullptr;
				pg.cluster = ~(0LU);
				pg.setRead(read);
				pg.setWrite(write);
				pg.setExecute(execute);
				pg.setValid(1);
			});
		}
	}
	else {
		//kernelSegment = KernelSystem::pInstance->sharedSegments[name];
		KernelProcess* proc = *kernelSegment->processes.begin();
		PageNum sharedSegmentPage;

		for (const auto& segment : proc->sharedSegments) {
			if (segment.second.pKernelSegment == kernelSegment) {
				sharedSegmentPage = segment.second.segment.pageStart;
				break;
			}
		}

		doOpToPagesInInterval(startPage, endPage, [&read, &write, &execute, &proc, &sharedSegmentPage](PageDescriptor& pg) {
			PageDescriptor& origPG = proc->getPageDescriptor(sharedSegmentPage);
			++sharedSegmentPage;
			pg.history = 0;
			pg.flags = 0;
			pg.flagsCoW = 0;
			pg.frame = origPG.frame;
			pg.cluster = origPG.cluster;
			pg.rootDescr = origPG.rootDescr;
			pg.setRead(read);
			pg.setWrite(write);
			pg.setExecute(execute);
			pg.setValid(1);
		});
	}

	sharedSegments.emplace(std::make_pair(name, SharedSegmentProc(startPage, endPage, kernelSegment)));
	kernelSegment->processes.push_back(this);

	return Status::OK;
}

template <class T>
void KernelProcess::doOpToPagesInInterval(PageNum startPage, PageNum endPage, T operation)
{
	unsigned short pmtOneStart = getPagePMTOneEntry(startPage);
	unsigned short pmtOneEnd = getPagePMTOneEntry(endPage);

	unsigned short pmtTwoStart = getPagePMTTwoEntry(startPage);
	unsigned short pmtTwoEnd = getPagePMTTwoEntry(endPage);

	if (pmtOneStart != pmtOneEnd) {
		PmtLevelTwo* pStartPmt = pmt->pPmtTable[pmtOneStart];

		for (unsigned short i = pmtTwoStart; i < PMT_TWO_PAGE_COUNT; ++i)
			operation(pStartPmt->pageDescr[i]);
		
		for (unsigned short i = pmtOneStart + 1; i < pmtOneEnd; ++i) {
			PmtLevelTwo* pPmtTwo = pmt->pPmtTable[i];
			for (unsigned short j = 0; j < PMT_TWO_PAGE_COUNT; ++j)
				operation(pPmtTwo->pageDescr[j]);
		}

		PmtLevelTwo* pEndPmt = pmt->pPmtTable[pmtOneEnd];
		
		for (unsigned short i = 0; i <= pmtTwoEnd; ++i)
			operation(pEndPmt->pageDescr[i]);
	}
	else {
		PmtLevelTwo* pPmtTwo = pmt->pPmtTable[pmtOneStart];
		for (unsigned short i = pmtTwoStart; i <= pmtTwoEnd; ++i)
			operation(pPmtTwo->pageDescr[i]);
	}

}

void KernelProcess::dumpProcessInfo()
{
	std::string procFileName = "dump\\proc_";
	std::stringstream procPID;
	procPID << pid << "_" << dumpCnter++;
	procFileName += procPID.str() + ".txt";
	std::ofstream dumpFile(procFileName);

	dumpFile << "Process PID: " << pid << std::endl;
	dumpFile << "Segment count: " << segments.size() << std::endl;
	for (unsigned short i = 0; i < segments.size(); ++i)
		dumpFile << "Segment: " << i << " " << segments[i].pageStart << " <-> " << segments[i].pageEnd << std::endl;
	dumpFile << "PMT One info:" << std::endl;
	for (unsigned short i = 0; i < PMT_ONE_TABLE_SIZE; ++i) {
		dumpFile << i << ": ";
		if (!pmt->pPmtTable[i])
			dumpFile << "-------" << std::endl;
		else
			dumpFile << "USED" << std::endl;
	}

	dumpFile << "PMT Two info: " << std::endl;
	for (int i = 0; i < PMT_ONE_TABLE_SIZE; ++i) {
		dumpFile << i << ": ";
		if (!pmt->pPmtTable[i])
			dumpFile << "-------" << std::endl;
		else {
			PmtLevelTwo* pmtTwo = pmt->pPmtTable[i];
			dumpFile << "USED" << std::endl;
			for (int j = 0; j < PMT_TWO_PAGE_COUNT; ++j) {
				const PageDescriptor& pg = pmtTwo->pageDescr[j];
				pg.print(dumpFile);
			}
		}
	}
}

void KernelProcess::freeLevelTwoPMT(unsigned short entry)
{
	KernelSystem::pInstance->freePmt(pmt->pPmtTable[entry]);
	pmt->pPmtTable[entry] = nullptr;
}

bool KernelProcess::isPageRangeOverlappingSegments(PageNum pageStart, PageNum pageEnd)
{
	for (const Segment& segment : segments)
		if (Segment::isSegmentOverlapping(segment, pageStart, pageEnd))
			return true;

	for (const auto& segment : sharedSegments)
		if (Segment::isSegmentOverlapping(segment.second.segment, pageStart, pageEnd))
			return true;

	return false;
}

unsigned short KernelProcess::getMissingPMTsInRange(PageNum pageStart, PageNum pageEnd)
{
	unsigned short pmtOneStart = getPagePMTOneEntry(pageStart);
	unsigned short pmtOneEnd = getPagePMTOneEntry(pageEnd);
	unsigned short missingPMTs = 0;
	for (unsigned short i = pmtOneStart; i <= pmtOneEnd; ++i)
		if (pmt->pPmtTable[i] == nullptr)
			++missingPMTs;

	return missingPMTs;
}

void KernelProcess::allocatMissingPMTsInRange(PageNum pageStart, PageNum pageEnd)
{
	unsigned short pmtOneStart = getPagePMTOneEntry(pageStart);
	unsigned short pmtOneEnd = getPagePMTOneEntry(pageEnd);
	for (unsigned short i = pmtOneStart; i <= pmtOneEnd; ++i)
		if (pmt->pPmtTable[i] == nullptr)
			pmt->pPmtTable[i] = KernelSystem::pInstance->getLevelTwoPmt();
}

PageDescriptor& KernelProcess::getPageDescriptor(PageNum page)
{
	return pmt->pPmtTable[getPagePMTOneEntry(page)]->pageDescr[getPagePMTTwoEntry(page)];
}

PhysicalAddress KernelProcess::getPhysicalAddress(VirtualAddress address, AccessType type)
{

	Status status = KernelSystem::pInstance->access(pid, address, type);

	if (status == Status::TRAP)
		throw "Fuck";
	if (status == Status::PAGE_FAULT) {
		Status s = KernelSystem::pInstance->pageFault(this, address);

		if (s != Status::OK)
			throw "Fuck";
	}
	
	return KernelSystem::pInstance->getPhysicalAddress(this, address);
}

unsigned char KernelProcess::readAddress(VirtualAddress address)
{
	PhysicalAddress addr = getPhysicalAddress(address, AccessType::READ);
	return *(unsigned char*)addr;
}

Status KernelProcess::disconnectSharedSegment(const char* name, bool isSegmentBeingDeleted)
{
	auto procSegmentIter = sharedSegments.find(name);
	if (procSegmentIter == sharedSegments.end()) {
		return Status::TRAP;
	}

	PageNum pageStart = procSegmentIter->second.segment.pageStart;
	PageNum pageEnd = procSegmentIter->second.segment.pageEnd;

	PageDescriptor& probe = getPageDescriptor(pageStart);
	SharedSegmentKernel* kernelSegment = procSegmentIter->second.pKernelSegment;
	//If the segment was CoW, it's being deleted once this process disconnects
	if (kernelSegment->processes.size() == 1 && kernelSegment->isCoW)
		isSegmentBeingDeleted = true;

	//Check if we were the root descriptor and the segment is being deleted
	if (probe.rootDescr == &probe && isSegmentBeingDeleted) {
		std::cout << "Last process on a shared segment disconnected.";
		doOpToPagesInInterval(pageStart, pageEnd, [](PageDescriptor& pg)
		{
			if (pg.isValidCluster())
				KernelSystem::pInstance->diskService.freeCluster(pg.cluster);
			if (pg.frame != nullptr)
				KernelSystem::pInstance->framePool.freeFrame(pg.frame);
		});

	}
	//Otherwise, check if we were the last proccess on a segment that is still available
	else if (kernelSegment->processes.size() == 1 && !isSegmentBeingDeleted) {
		std::cout << "Last process from a live segment disconnected.";
		if (dumpSharedSegmentToDisk(kernelSegment, pageStart, pageEnd) != Status::OK) {
			return Status::TRAP;
		}
	}
	else if (probe.rootDescr == &probe) {
		std::cout << "Root disconnected.";
		selectNewSharedSegmentRoot(kernelSegment, pageStart, pageEnd);
	}
	else
		std::cout << "Non important process disconnected.";

	//Check valid frames and see if any of their descriptors are set to us, and if yes, change them
	if (kernelSegment->processes.size() != 1 && !isSegmentBeingDeleted) {
		doOpToPagesInInterval(procSegmentIter->second.segment.pageStart, procSegmentIter->second.segment.pageEnd, [](PageDescriptor& pg)
		{
			if (pg.rootDescr->frame)
				if (pg.rootDescr->frame->pageDescr == &pg)
					pg.rootDescr->frame->pageDescr = pg.rootDescr;
		});
	}
	//Deallocate the pmts and set them to invalid
	unsigned short pmtOneStart = getPagePMTOneEntry(pageStart);
	unsigned short pmtTwoStart = getPagePMTTwoEntry(pageStart);
	unsigned short pmtOneEnd = getPagePMTOneEntry(pageEnd);
	unsigned short pmtTwoEnd = getPagePMTTwoEntry(pageEnd);
	
	for (unsigned short i = pmtOneStart + 1; i < pmtOneEnd; ++i)
		freeLevelTwoPMT(i);

	if (pmtOneStart == pmtOneEnd) {
		if (pmtTwoStart == 0 && pmtTwoEnd == PMT_TWO_PAGE_COUNT - 1)
			freeLevelTwoPMT(pmtOneStart);
		else {
			doOpToPagesInInterval(pageStart, pageEnd, [](PageDescriptor& pg) {
				pg.setValid(0);
				}
			);
		}
	}
	else {
		for (unsigned short i = pmtTwoStart; i < PMT_TWO_PAGE_COUNT; ++i)
			pmt->pPmtTable[pmtOneStart]->pageDescr[i].setValid(0);
		
		if (!pmt->pPmtTable[pmtOneStart]->hasValidPages())
			freeLevelTwoPMT(pmtOneStart);

		for (unsigned short i = 0; i <= pmtTwoEnd; ++i)
				pmt->pPmtTable[pmtOneEnd]->pageDescr[i].setValid(0);
		if (!pmt->pPmtTable[pmtOneEnd]->hasValidPages())
			freeLevelTwoPMT(pmtOneEnd);
	}

	sharedSegments.erase(name);
	for (auto iter = kernelSegment->processes.begin(); iter != kernelSegment->processes.end(); ++iter) {
		if ((*iter)->pid == pid) {
			kernelSegment->processes.erase(iter);
			break;
		}
	}

	//The last process disconnected from a CoW shared segment
	if (kernelSegment->isCoW && kernelSegment->processes.size() == 0) {
		KernelSystem::pInstance->sharedSegments.erase(kernelSegment->name);
		delete kernelSegment;
	}
	
	return Status::OK;
}

Status KernelProcess::deleteSharedSegment(const char* name)
{
	auto procSegmentIter = sharedSegments.find(name);
	if (procSegmentIter == sharedSegments.end())
		return Status::TRAP;

	SharedSegmentKernel* kernelSegment = procSegmentIter->second.pKernelSegment;
	while (kernelSegment->processes.size()) {
		KernelProcess* proc = kernelSegment->processes.front();
		proc->disconnectSharedSegment(name, 1); //Will always work
	}
	KernelSystem::pInstance->sharedSegments.erase(kernelSegment->name);
	delete kernelSegment;

	return Status::OK;
}

void KernelProcess::writeAddress(VirtualAddress address, unsigned char data)
{
	PhysicalAddress addr = getPhysicalAddress(address, AccessType::WRITE);
	*(unsigned char*)addr = data;
}

void KernelProcess::selectNewSharedSegmentRoot(SharedSegmentKernel* kernelSegment, PageNum pageStart, PageNum pageEnd)
{
	auto nextProc = kernelSegment->processes.begin();
	while ((*nextProc)->pid == pid)
		++nextProc;

	//Find a new process which becomes this segment's root and update the values in it's descriptors
	KernelProcess* newRootProc = *nextProc;
	PageNum pageIter = pageStart;
	auto& newRootSegment = newRootProc->sharedSegments.find(kernelSegment->name)->second.segment;
	newRootProc->doOpToPagesInInterval(newRootSegment.pageStart, newRootSegment.pageEnd,
		[this, &pageIter](PageDescriptor& pg) {
		PageDescriptor& oldRootPg = this->getPageDescriptor(pageIter);
		pageIter++;
		pg.cluster = oldRootPg.cluster;
		pg.frame = oldRootPg.frame;
	}
	);

	//Update the root descriptors of other segment proccesses
	auto end = kernelSegment->processes.end();
	for (auto procIter = kernelSegment->processes.begin(); procIter != end; ++procIter) {
		KernelProcess* proc = *procIter;
		auto segmentDescr = proc->sharedSegments.find(kernelSegment->name)->second.segment;
		PageNum rootPageIter = newRootSegment.pageStart;
		proc->doOpToPagesInInterval(segmentDescr.pageStart, segmentDescr.pageEnd,
			[&rootPageIter, &newRootProc](PageDescriptor& pg) {
			PageDescriptor& rootPG = newRootProc->getPageDescriptor(rootPageIter);
			++rootPageIter;
			pg.rootDescr = &rootPG;
		}
		);
	}
}

Status KernelProcess::dumpSharedSegmentToDisk(SharedSegmentKernel* kernelSegment, PageNum pageStart, PageNum pageEnd)
{
	//Check if we have enough pmt storage for book-keeping
	unsigned dumpSize = kernelSegment->size / SHARED_SEGMENT_CLUSTERS_PER_DUMP;
	if (kernelSegment->size % SHARED_SEGMENT_CLUSTERS_PER_DUMP != 0)
		++dumpSize;

	if (KernelSystem::pInstance->getFreePmtBlocks() < dumpSize)
		return Status::TRAP;

	//Check how many disk clusters we are missing
	unsigned missingDiskClusters = 0;
	doOpToPagesInInterval(pageStart, pageEnd, [&missingDiskClusters](PageDescriptor& pg) {
		if (pg.frame && !pg.isValidCluster())
			++missingDiskClusters;
	});

	if (missingDiskClusters > KernelSystem::pInstance->diskService.getFreeClusterCount())
		return Status::TRAP;

	SharedSegmentDiskDump** dump = kernelSegment->diskDump = new SharedSegmentDiskDump*[dumpSize];
	for (unsigned i = 0; i < dumpSize; ++i)
		dump[i] = (SharedSegmentDiskDump*)KernelSystem::pInstance->getLevelOnePmt(); //Why would this be a bad idea? :D
	
	PageNum pageIter = 0;

	doOpToPagesInInterval(pageStart, pageEnd, [&dump, &pageIter](PageDescriptor& pg)
	{
		if (pg.frame != nullptr) {
			if (!pg.isValidCluster())
				pg.cluster = KernelSystem::pInstance->diskService.getFreeCluster();
			
			if (pg.frame->getDirty()) {
				KernelSystem::pInstance->diskService.writePageToDisk(pg);
				pg.frame->setDirty(0);
			}

			KernelSystem::pInstance->framePool.freeFrame(pg.frame);
			
		}

		dump[pageIter / SHARED_SEGMENT_CLUSTERS_PER_DUMP]->pageCluster[pageIter % SHARED_SEGMENT_CLUSTERS_PER_DUMP] = pg.cluster;
		++pageIter;
	});

	return Status::OK;
}

Process* KernelProcess::clone()
{
	unsigned long neededPMTCnt = 1; //One for level one pmt
	for (unsigned short i = 0; i < PMT_ONE_TABLE_SIZE; ++i)
		if (pmt->pPmtTable[i] != nullptr)
			++neededPMTCnt;

	if (KernelSystem::pInstance->getFreePmtBlocks() < neededPMTCnt)
		return nullptr;

	Process* retProc = KernelSystem::pInstance->createProcess();
	KernelProcess* cloneProc = retProc->pProcess;

	//Convert read/execute segments into CoW shared segments
	convertCompatibleSegmentsToCoWSharedSegments();

	cloneProc->pmt = KernelSystem::pInstance->getLevelOnePmt();
	for (auto& segmentIter : sharedSegments) {
		auto sharedSegment = segmentIter.second;
		AccessType rights = getSegmentAccessRights(sharedSegment.segment);
		bool isCoW = sharedSegment.pKernelSegment->isCoW;
		cloneProc->createSharedSegment(sharedSegment.segment.pageStart << OFFSET_BITS, sharedSegment.pKernelSegment->size, sharedSegment.pKernelSegment->name.c_str(), rights, isCoW);
	}
	
	//The rest of the segments have to be chained into CoW lists
	for (auto& seg : segments) {
		AccessType rights = seg.accessRights;
		cloneProc->createSegment(seg.pageStart << OFFSET_BITS, seg.pageEnd - seg.pageStart + 1, rights);
		for (PageNum page = seg.pageStart; page <= seg.pageEnd; ++page) {
			PageDescriptor& myPg = getPageDescriptor(page);
			PageDescriptor& clonePg = cloneProc->getPageDescriptor(page);
			clonePg.convertToCoW();
			if (!myPg.isCoW())
				myPg.convertToCoW();

			if (myPg.isNextPidValid()) {
				KernelProcess* nextProc = KernelSystem::pInstance->processes[myPg.cowList.nextProcPid];
				PageDescriptor& nextPg = nextProc->getPageDescriptor(page);
				clonePg.cowList.nextProcPid = myPg.cowList.nextProcPid;
				nextPg.cowList.prevProcPid = cloneProc->pid;
			}

			myPg.cowList.nextProcPid = cloneProc->pid;
			clonePg.cowList.prevProcPid = pid;

			clonePg.cluster = myPg.cluster;
			clonePg.frame = myPg.frame;
			if (myPg.frame)
				myPg.frame->pageNo = (unsigned short)page;
		}
	}

	return retProc;
}

AccessType KernelProcess::getSegmentAccessRights(Segment& segment)
{
	PageDescriptor& probe = getPageDescriptor(segment.pageStart);
	unsigned char read = probe.getRead();
	unsigned char write = probe.getWrite();
	unsigned char execute = probe.getExecute();
	return convertRWXToAccessType(read, write, execute);
}

AccessType KernelProcess::convertRWXToAccessType(unsigned char read, unsigned char write, unsigned char execute)
{
	if (read && write)
		return AccessType::READ_WRITE;
	if (read)
		return AccessType::READ;
	if (write)
		return AccessType::WRITE;
	if (execute)
		return AccessType::EXECUTE;

	return AccessType::READ;//Quell the compiler warnings
}

void KernelProcess::convertCompatibleSegmentsToCoWSharedSegments()
{
	const std::string COW_SEGMENT_NAME_PREFIX = "__SYS_COW_SEG__";
	unsigned short i = 0;
	while (i < segments.size()) {
		auto& seg = segments[i];
		AccessType rights = seg.accessRights;
		//Only read and execute segments can be converted to CoW shared segments
		if (rights == AccessType::READ || rights == AccessType::EXECUTE) {
			std::stringstream converter;
			converter << COW_SEGMENT_NAME_PREFIX << KernelSystem::pInstance->cowSegmentNameIdCounter;
			std::string name = converter.str();

			++KernelSystem::pInstance->cowSegmentNameIdCounter;

			SharedSegmentKernel* kernelSegment = new SharedSegmentKernel(name, seg.pageEnd - seg.pageStart + 1, true);
			kernelSegment->processes.push_back(this);
			sharedSegments.emplace(name, SharedSegmentProc(seg.pageStart, seg.pageEnd, kernelSegment));
			doOpToPagesInInterval(seg.pageStart, seg.pageEnd, [](PageDescriptor& pg) {
				pg.rootDescr = &pg;
			});

			//Seg is invalid past this point
			segments.erase(segments.begin() + i);
		}
		else
			++i;
	}
}

void KernelProcess::deletePageFromCoWList(PageNum page)
{
	PageDescriptor& pg = getPageDescriptor(page);
	KernelProcess* nextProc = nullptr;
	KernelProcess* prevProc = nullptr;
	PageDescriptor* nextPg = nullptr;
	PageDescriptor* prevPg = nullptr;
	if (pg.isPrevPidValid()) {
		prevProc = KernelSystem::pInstance->processes[pg.cowList.prevProcPid];
		prevPg = &prevProc->getPageDescriptor(page);
	}

	if (pg.isNextPidValid()) {
		nextProc = KernelSystem::pInstance->processes[pg.cowList.nextProcPid];
		nextPg = &nextProc->getPageDescriptor(page);
	}

	if (prevPg)
		prevPg->cowList.nextProcPid = pg.cowList.nextProcPid;
	if (nextPg)
		nextPg->cowList.prevProcPid = pg.cowList.prevProcPid;

	//One of them must be valid
	PageDescriptor* randomPg = nextPg;
	if (!randomPg)
		randomPg = prevPg;

	if (!randomPg->isNextPidValid() && !randomPg->isPrevPidValid()) 
		randomPg->restoreFlagsPreCoW();
	
	pg.restoreFlagsPreCoW();
}