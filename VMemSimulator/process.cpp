#include "process.h"
#include "kernelproc.h"
#include "kernelsys.h"

Process::Process(ProcessId pid)
	: pProcess(new KernelProcess(pid))
{
}

Process::~Process()
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	delete pProcess;
}

ProcessId Process::getProcessId() const
{
	return pProcess->getPid();
}

Status Process::createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pProcess->createSegment(startAddress, segmentSize, flags);
}

Status Process::loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags, void * content)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pProcess->loadSegment(startAddress, segmentSize, flags, content);
}

Status Process::deleteSegment(VirtualAddress startAddress)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pProcess->deleteSegment(startAddress);
}

Status Process::pageFault(VirtualAddress address)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return KernelSystem::pInstance->pageFault(pProcess, address);
}

PhysicalAddress Process::getPhysicalAddress(VirtualAddress address)
{
	return KernelSystem::pInstance->getPhysicalAddress(pProcess, address);
}

Status Process::createSharedSegment(VirtualAddress startAddress, PageNum segmentSize, const char * name, AccessRight flags)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pProcess->createSharedSegment(startAddress, segmentSize, name, flags);
}

Status Process::disconnectSharedSegment(const char* name)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pProcess->disconnectSharedSegment(name, false);
}

Status Process::deleteSharedSegment(const char* name)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pProcess->deleteSharedSegment(name);
}

Process* Process::clone(ProcessId pid)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	ProcessId expectedPid = KernelSystem::pInstance->freePidCnt;
	if (!KernelSystem::pInstance->freePidStack.empty())
		expectedPid = KernelSystem::pInstance->freePidStack.top();
	if (pid != expectedPid) //Someone tried to select their pid
		return nullptr;

	return KernelSystem::pInstance->cloneProcess(pProcess->pid);
}