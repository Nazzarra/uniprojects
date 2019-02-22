#include "system.h"
#include "kernelsys.h"

System::System(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace, PageNum pmtSpaceSize, Partition * partition)
{
	pSystem = KernelSystem::pInstance = new KernelSystem(processVMSpace, processVMSpaceSize, pmtSpace, pmtSpaceSize, partition);
}

System::~System()
{
	delete pSystem;
}

Process * System::createProcess()
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pSystem->createProcess();
}

Time System::periodicJob()
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pSystem->periodicJob();
}

Status System::access(ProcessId pid, VirtualAddress address, AccessType type)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return pSystem->access(pid, address, type);
}

Process* System::cloneProcess(ProcessId pid)
{
	std::lock_guard<std::mutex> lock(KernelSystem::pInstance->systemLock);
	return KernelSystem::pInstance->cloneProcess(pid);
}