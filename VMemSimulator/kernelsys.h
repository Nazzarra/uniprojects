#ifndef _VM_KERNEL_SYSTEM
#define _VM_KERNEL_SYSTEM

#include "vm_declarations.h"
#include "slaballocator.h"
#include "pmt.h"
#include "framepool.h"
#include "diskservice.h"

#include <unordered_map>
#include <stack>
#include <mutex>

class Partition;
class KernelProcess;
class Process;
class KernelSystem
{
public:
	PmtLevelOne* getLevelOnePmt();
	PmtLevelTwo* getLevelTwoPmt();
	void freePmt(PhysicalAddress pmt);
	unsigned long getFreePmtBlocks() const;

	static KernelSystem* pInstance;
	static const Time PERIOD_JOB_CALL_TIME = 100000;

	const PhysicalAddress frameSpace;
	const PageNum totalFrameCount;

	const PhysicalAddress pmtSpace;
	const PageNum pmtSpaceSize;

	const Partition* partition;

	Process* createProcess();
	void killProcess(KernelProcess* proc);

	Status access(ProcessId pid, VirtualAddress address, AccessType type);

	Status pageFault(KernelProcess* proc, VirtualAddress address);

	PhysicalAddress getPhysicalAddress(KernelProcess* proc, VirtualAddress address);

	Time periodicJob();

	Process* cloneProcess(ProcessId pid);

	static void dumpProcessInfo(Process* p);

	unsigned long getVictimPageFrame();
	Status releaseFrame(unsigned long frameNo);

	SlabAllocator pmtAllocator;
	DiskService diskService;
	FramePool framePool;

	std::vector<KernelProcess*> processes;
	std::unordered_map<std::string, SharedSegmentKernel*> sharedSegments;
	ProcessId freePidCnt;
	std::stack<ProcessId> freePidStack;

	std::mutex systemLock;

	unsigned long cowSegmentNameIdCounter = 0;

	KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace, PageNum pmtSpaceSize, Partition * partition);
	friend class System;
	friend class KernelProcess;
};

#endif