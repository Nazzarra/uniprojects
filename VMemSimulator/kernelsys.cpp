#include "kernelsys.h"
#include <cassert>
#include "process.h"
#include "kernelproc.h"
#include <cstring>
#include "pmt.h"
#include <iostream>
#include <chrono>
#include <cstdlib>

KernelSystem* KernelSystem::pInstance = nullptr;

unsigned long GLOBAL_PF_CNT = 0;

KernelSystem::KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace, PageNum pmtSpaceSize, Partition * partition)
	: frameSpace(processVMSpace), totalFrameCount(processVMSpaceSize), pmtSpace(pmtSpace), pmtSpaceSize(pmtSpaceSize), pmtAllocator(pmtSpace, pmtSpaceSize), framePool(processVMSpace, processVMSpaceSize), diskService(partition), freePidCnt(0)
{
	assert(pInstance == nullptr);
	pInstance = this;
}

PmtLevelOne* KernelSystem::getLevelOnePmt() 
{
	PmtLevelOne* ret = (PmtLevelOne*)pmtAllocator.getSlot();
	for (int i = 0; i < PMT_ONE_TABLE_SIZE; ++i)
		ret->pPmtTable[i] = nullptr;

	return ret;
}

PmtLevelTwo* KernelSystem::getLevelTwoPmt()
{
	PmtLevelTwo* ret = (PmtLevelTwo*)pmtAllocator.getSlot();
	ret->init();
	return ret;
}

void KernelSystem::freePmt(PhysicalAddress pmt)
{
	pmtAllocator.freeSlot(pmt);
}

unsigned long KernelSystem::getFreePmtBlocks() const
{
	return pmtAllocator.getFreeSlotCount();
}

Process* KernelSystem::createProcess()
{
	PmtLevelOne* pmtOne = getLevelOnePmt();
	if (!pmtOne)
		return nullptr;

	ProcessId assignedPid;
	if (!freePidStack.empty()) {
		assignedPid = freePidStack.top();
		freePidStack.pop();
	}
	else
		assignedPid = freePidCnt++;

	Process* proc = new Process(assignedPid);
	proc->pProcess->pmt = pmtOne;
	if (assignedPid >= processes.size())
		processes.push_back(proc->pProcess);
	else
		processes[assignedPid] = proc->pProcess;

	return proc;
}

void KernelSystem::killProcess(KernelProcess* proc)
{
	while (!proc->segments.empty())
		proc->deleteSegment(proc->segments.begin()->pageStart << OFFSET_BITS);

	while (proc->sharedSegments.size() != 0) {
		proc->disconnectSharedSegment(proc->sharedSegments.begin()->second.pKernelSegment->name.c_str());
	}

	processes[proc->getPid()] = nullptr;
}

Status KernelSystem::access(ProcessId pid, VirtualAddress address, AccessType type)
{
	KernelProcess* proc = processes[pid];
	if (!proc)
		return Status::TRAP;
	
	unsigned short pmtOneEntry = getPMTOneEntry(address);
	PmtLevelTwo* pmtTwo = proc->pmt->pPmtTable[pmtOneEntry];
	if (!pmtTwo)
		return Status::TRAP;

	unsigned short pmtTwoEntry = getPMTTwoEntry(address);
	PageDescriptor& pg = pmtTwo->pageDescr[pmtTwoEntry];

	if (!pg.getValid())
		return Status::TRAP;

	PageDescriptor& rootPG = pg.isShared() ? *pg.rootDescr : pg;

	unsigned char read = getReadFlag(type);
	unsigned char write = getWriteFlag(type);
	unsigned char execute = getExecuteFlag(type);
	if (read && !pg.getRead())
		return Status::TRAP;
	if (write && !pg.getWrite()) {
		if (pg.isCoW()) {
			pg.setDirty(1);
			pg.setReferenced(1);
			return Status::PAGE_FAULT;
		}
		return Status::TRAP;
	}
	if (execute && !pg.getExecute())
		return Status::TRAP;

	pg.setReferenced(1);

	if (!rootPG.frame) {
		if (write) {
			pg.setDirty(1);
		}

		return Status::PAGE_FAULT;
	}
	
	if (write) {
		rootPG.frame->setDirty(1);
		rootPG.frame->setRef(1);
	}

	return Status::OK;
}

Status KernelSystem::pageFault(KernelProcess* proc, VirtualAddress address)
{
	unsigned short pmtOneEntry = getPMTOneEntry(address);
	if (!proc->pmt->pPmtTable[pmtOneEntry])
		return Status::TRAP;

	PageDescriptor& pg = proc->getPageDescriptor(getVAPage(address));
	if (!pg.getValid())
		return Status::TRAP;

	if(!framePool.hasFreeFrames()) {
		unsigned long victim = getVictimPageFrame();
		if (releaseFrame(victim) != Status::OK)
			return Status::TRAP;
	}

	FrameDescr* frame = framePool.getFreeFrame();
	PageDescriptor& rootPG = pg.isShared() ? *pg.rootDescr : pg;
	FrameDescr* oldFrame = pg.frame;
	frame->pageDescr = &rootPG;
	rootPG.frame = frame;

	bool wasCoWWithNewPage = pg.isCoW() && pg.getDirty();

	if (wasCoWWithNewPage && oldFrame) {
		memcpy(pg.frame->frameStart, oldFrame->frameStart, PAGE_SIZE);
		pg.cluster = ~(0LU);
	}
	else if (rootPG.isValidCluster()) {
		diskService.readPageFromDisk(rootPG);
		if (pg.getDirty()) {
			frame->setDirty(1);
			pg.setDirty(0);
		}
		if (wasCoWWithNewPage)
			pg.cluster = ~(0LU);
	}
	else {
		frame->setDirty(1);
		pg.setDirty(0);
	}

	if (wasCoWWithNewPage) {
		if (oldFrame) 
			if (oldFrame->pageDescr == &pg) {
				KernelProcess* nextProc = nullptr;
				if (pg.isNextPidValid())
					nextProc = processes[pg.cowList.nextProcPid];
				else
					nextProc = processes[pg.cowList.prevProcPid];
				oldFrame->pageDescr = &nextProc->getPageDescriptor(getVAPage(address));
			}
		
		proc->deletePageFromCoWList(getVAPage(address));
	}

	if (pg.isCoW()) {
		PageDescriptor* currPg = &pg;
		while (currPg->isPrevPidValid()) {
			currPg = &KernelSystem::pInstance->processes[currPg->cowList.prevProcPid]->getPageDescriptor(getVAPage(address));
			currPg->frame = frame;
		}
		currPg = &pg;
		while (currPg->isNextPidValid()) {
			currPg = &KernelSystem::pInstance->processes[currPg->cowList.nextProcPid]->getPageDescriptor(getVAPage(address));
			currPg->frame = frame;
		}
		pg.frame->pageNo = getVAPage(address);
	}

	rootPG.frame->setRef(1);
	return Status::OK;
}

Status KernelSystem::releaseFrame(unsigned long frameNo)
{
	PageDescriptor& pg = *framePool.frames[frameNo].pageDescr;
	PageDescriptor& rootPG = pg.isShared() ? *pg.rootDescr : pg;

	ClusterNo cluster = ~(0LU);
	if (!rootPG.isValidCluster()) {
		if (!diskService.hasFreeClusters())
			return Status::TRAP;

		cluster = diskService.getFreeCluster();
		rootPG.cluster = cluster;
	}

	if (rootPG.frame->getDirty()) {
		diskService.writePageToDisk(pg);
		rootPG.frame->setDirty(0);
	}

	FrameDescr* frame = rootPG.frame;

	if (pg.isCoW()) {
		PageDescriptor* currPg = &pg;
		while (currPg->isPrevPidValid()) {
			currPg = &KernelSystem::pInstance->processes[currPg->cowList.prevProcPid]->getPageDescriptor(frame->pageNo);
			currPg->frame = nullptr;
			if (cluster != ~(0LU)) 
				currPg->cluster = cluster;
		}
		currPg = &pg;
		while (currPg->isNextPidValid()) {
			currPg = &KernelSystem::pInstance->processes[currPg->cowList.nextProcPid]->getPageDescriptor(frame->pageNo);
			currPg->frame = nullptr;
			if (cluster != ~(0LU))
				currPg->cluster = cluster;
		}
	}

	rootPG.frame = nullptr;
	framePool.freeFrame(frame);

	return Status::OK;
}

Time KernelSystem::periodicJob()
{
	for (auto& proc : processes)
	{
		if (proc) {
			for (unsigned short i = 0; i < PMT_ONE_TABLE_SIZE; ++i) {
				PmtLevelTwo* pPmtTwo = proc->pmt->pPmtTable[i];
				if (pPmtTwo)
					for (unsigned short j = 0; j < PMT_TWO_PAGE_COUNT; ++j) {
						PageDescriptor& pg = pPmtTwo->pageDescr[j];
						if (pg.getValid()) {
							pg.history = (pg.history >> 1) | (pg.getReferenced() << (sizeof(pg.history) * 8 - 1));
							pg.setReferenced(0);
						}
					}
			}
		}
	}
	return 0;
	//return PERIOD_JOB_CALL_TIME;
}

unsigned long KernelSystem::getVictimPageFrame()
{
	/*
	unsigned long leastReferenced = ~(0LU);
	unsigned long leastReferencedIndex = leastReferenced;
	for (unsigned long i = 0; i < totalFrameCount; ++i) {
		if (framePool.frames[i].getValid()) {
			if (framePool.frames[i].pageDescr->history < leastReferenced) {
				leastReferenced = framePool.frames[i].pageDescr->history;
				leastReferencedIndex = i;
			}
		}
	}

	if (leastReferenced == ~(0LU))
		throw "Sheeeeeit!";

	return leastReferencedIndex;
	*/
	static unsigned long clockHand = 0;
	while (true) {
		if (framePool.frames[clockHand].getValid()) {
			if (!framePool.frames[clockHand].getRef())
				return clockHand;

			framePool.frames[clockHand].setRef(0);
		}
		clockHand = (clockHand + 1) % totalFrameCount;
	}
}

void KernelSystem::dumpProcessInfo(Process* p)
{
	p->pProcess->dumpProcessInfo();
}

PhysicalAddress KernelSystem::getPhysicalAddress(KernelProcess* pProcess, VirtualAddress address)
{
	PmtLevelTwo* pmtTwo = pProcess->pmt->pPmtTable[getPMTOneEntry(address)];
	if (!pmtTwo)
		return nullptr;

	PageDescriptor& pg = pmtTwo->pageDescr[getPMTTwoEntry(address)];
	PageDescriptor& rootPG = pg.isShared() ? *pg.rootDescr : pg;
	if (!rootPG.getValid())
		return nullptr;
	if (!rootPG.frame)
		return nullptr;

	return (unsigned char*)rootPG.frame->frameStart + (unsigned long)(address & OFFSET_MASK);
}

Process* KernelSystem::cloneProcess(ProcessId pid)
{
	return processes[pid]->clone();
}