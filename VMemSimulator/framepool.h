#ifndef _VM_FRAME_POOL_H
#define _VM_FRAME_POOL_H

#include "vm_declarations.h"
#include "slaballocator.h"
#include "pmt.h"

constexpr unsigned char FR_FL_DIRTY_MASK = 1;
constexpr unsigned char FR_FL_VALID_MASK = 2;
constexpr unsigned char FR_FL_REF_MASK = 4;

constexpr unsigned char FR_FL_DIRTY_BIT = 0;
constexpr unsigned char FR_FL_VALID_BIT = 1;
constexpr unsigned char FR_FL_REF_BIT = 2;

struct FrameDescr
{
	union {
		PageDescriptor* pageDescr;
		FrameDescr* pNextFreeFrame;
	};

	PhysicalAddress frameStart;
	unsigned short pageNo; // 2^24 virtual memory, 2^14 pages, this works :D
	unsigned char flags; //ref, valid, dirty
	FrameDescr() : pageDescr(nullptr), pageNo(0), flags(0) {}

	void setDirty(unsigned char dirty)
	{
		flags = (flags & ~FR_FL_DIRTY_MASK) | dirty;
	}

	unsigned char getDirty()
	{
		return flags & FR_FL_DIRTY_MASK;
	}

	void setValid(unsigned char valid)
	{
		flags = (flags & ~FR_FL_VALID_MASK) | (valid << 1);
	}

	unsigned char getValid()
	{
		return (flags & FR_FL_VALID_MASK) >> 1;
	}

	void setRef(unsigned char ref)
	{
		flags = (flags & ~FR_FL_REF_MASK) | (ref << FR_FL_REF_BIT);
	}

	unsigned char getRef()
	{
		return (flags & FR_FL_REF_MASK) >> FR_FL_REF_BIT;
	}
};

class FramePool
{
public:
	FramePool(PhysicalAddress memory, PageNum memorySize);
	~FramePool();

	FrameDescr* getFreeFrame();
	void freeFrame(FrameDescr* frame);
	
	bool hasFreeFrames();

	void dumpFrameMemory();

private:
	unsigned long dumpCnt = 0;
	SlabAllocator frameAllocator;
	FrameDescr* frames;
	FrameDescr* pFreeFramesHead;

	friend class KernelSystem;
};


#endif