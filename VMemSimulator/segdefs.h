#ifndef _VM_SEGMENT_DEFS
#define _VM_SEGMENT_DEFS

#include <string>
#include <unordered_map>
#include "vm_declarations.h"
#include <cassert>

class KernelProcess;

struct Segment
{
	PageNum pageStart, pageEnd;
	AccessType accessRights;

	Segment(PageNum pageStart, PageNum pageEnd, AccessType accessRights) : pageStart(pageStart), pageEnd(pageEnd), accessRights(accessRights) {}

	static bool isSegmentOverlapping(const Segment& segment, PageNum pageStart, PageNum pageEnd)
	{
		if (segment.pageStart >= pageStart && segment.pageEnd <= pageEnd)
			return true;

		if (segment.pageStart <= pageStart && segment.pageEnd >= pageStart)
			return true;

		return segment.pageStart <= pageEnd && segment.pageEnd >= pageEnd;
	}
};

struct SharedSegmentKernel
{
	std::string name;
	std::vector<KernelProcess*> processes;
	PageNum size;
	SharedSegmentDiskDump** diskDump;
	bool isCoW;

	SharedSegmentKernel(const std::string& name, PageNum size, bool isCoW = false) : name(name), size(size), diskDump(nullptr), isCoW(isCoW){}
};

struct SharedSegmentProc
{
	Segment segment;
	SharedSegmentKernel* pKernelSegment;

	//To keep STL containers happy
	SharedSegmentProc()
		: segment(0, 0, AccessType::READ_WRITE)
	{
		assert(false);
	}

	SharedSegmentProc(PageNum pageStart, PageNum pageEnd, SharedSegmentKernel* pKernelSegment) : segment(pageStart, pageEnd, AccessType::READ_WRITE), pKernelSegment(pKernelSegment){}
};


#endif