#ifndef _VM_SLAB_ALLOCATOR
#define _VM_SLAB_ALLOCATOR

#include "vm_declarations.h"

constexpr unsigned long SLOT_SIZE = PAGE_SIZE;

class SlabAllocator 
{
private:
	struct Slot 
	{
		Slot* pNext;
	};

public:
	SlabAllocator(PhysicalAddress mem, unsigned long slotCount);

	PhysicalAddress getSlot();
	void freeSlot(PhysicalAddress slot);

	unsigned long getSlotCount() const;
	unsigned long getFreeSlotCount() const;
	unsigned long getSlotNo(PhysicalAddress slot) const;

private:
	PhysicalAddress mem;
	unsigned long slotCount, freeSlotCount;
	Slot* pHead;
};

#endif