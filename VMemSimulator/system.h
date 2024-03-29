#ifndef _VM_SYSTEM_H
#define _VM_SYSTEM_H

#include "vm_declarations.h" 
class Partition;
class Process;
class KernelProcess;
class KernelSystem;

class System {
public:
	System(PhysicalAddress processVMSpace, PageNum processVMSpaceSize,
		PhysicalAddress pmtSpace, PageNum pmtSpaceSize,
		Partition* partition);

	~System();
	Process* createProcess();
	
	Time periodicJob();
	// Hardware job 
	Status access(ProcessId pid, VirtualAddress address, AccessType type);

	Process* cloneProcess(ProcessId pid);
private:
	KernelSystem *pSystem;
	friend class Process;
	friend class KernelProcess;
};

#endif