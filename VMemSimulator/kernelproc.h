#ifndef _VM_KERNEL_PROC
#define _VM_KERNEL_PROC

#include <string>
#include <vector>
#include "vm_declarations.h"
#include "pmt.h"
#include "segdefs.h"
#include <unordered_map>

class Process;
class KernelProcess
{
public:
	KernelProcess(ProcessId pid);
	~KernelProcess();
	
	ProcessId getPid() const;

	Status createSegment(VirtualAddress startAddress, PageNum segmentSize,
		AccessRight flags);

	Status loadSegment(VirtualAddress startAddress, PageNum segmentSize,
		AccessRight flags, void* content);

	Status deleteSegment(VirtualAddress startAddress);

	Status createSharedSegment(VirtualAddress startAddress,
		PageNum segmentSize, const char* name, AccessRight flags, bool isCoWSegment = false);

	Status disconnectSharedSegment(const char* name, bool isSegmentBeingDeleted = false);

	Status deleteSharedSegment(const char* name);

	Process* clone();

	template <class T>
	void doOpToPagesInInterval(PageNum startPage, PageNum endPage, T operation);

	void dumpProcessInfo();
	void freeLevelTwoPMT(unsigned short entry);
	bool isPageRangeOverlappingSegments(PageNum startPage, PageNum endPage);
	unsigned short getMissingPMTsInRange(PageNum startPage, PageNum endPage);
	void allocatMissingPMTsInRange(PageNum startPage, PageNum endPage);
	PageDescriptor& getPageDescriptor(PageNum page);

	void selectNewSharedSegmentRoot(SharedSegmentKernel* kernelSegment, PageNum startPage, PageNum endPage);
	Status dumpSharedSegmentToDisk(SharedSegmentKernel* kernelSegment, PageNum startPage, PageNum endPage);

	AccessType getSegmentAccessRights(Segment& segment);
	//AccessType getSegmentCoWAccessRights(Segment& segment);

	void convertCompatibleSegmentsToCoWSharedSegments();
	void deletePageFromCoWList(PageNum page);

	static AccessType convertRWXToAccessType(unsigned char read, unsigned char write, unsigned char execute);

	/* DBG */
	unsigned char readAddress(VirtualAddress address);
	void writeAddress(VirtualAddress address, unsigned char data);

	PhysicalAddress getPhysicalAddress(VirtualAddress address, AccessType accessType);

	ProcessId pid;
	PmtLevelOne* pmt;
	std::vector<Segment> segments;
	std::unordered_map<std::string, SharedSegmentProc> sharedSegments;
	
	unsigned long dumpCnter = 0;

	friend class KernelSystem;
	friend class Process;
};

#endif