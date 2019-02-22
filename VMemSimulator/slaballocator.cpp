#include "slaballocator.h"

SlabAllocator::SlabAllocator(PhysicalAddress mem, unsigned long slotCount)
	: mem(mem), slotCount(slotCount), freeSlotCount(slotCount), pHead(nullptr)
{
	for (unsigned long i = 0; i < slotCount; ++i) {
		Slot* pSlot = (Slot*)((unsigned char*)mem + (slotCount - 1 - i) * SLOT_SIZE);
		pSlot->pNext = pHead;
		pHead = pSlot;
	}
}

PhysicalAddress SlabAllocator::getSlot() 
{
	if (pHead) {
		Slot* pSlot = pHead;
		pHead = pHead->pNext;
		--freeSlotCount;
		return (PhysicalAddress)pSlot;//PA is void pointer, cast not necessary
	}

	return nullptr;
}

void SlabAllocator::freeSlot(PhysicalAddress addr) 
{
	//As the class is used internally, slots are guaranteed to be from the same slab
	((Slot*)addr)->pNext = pHead;
	pHead = (Slot*)addr;
	++freeSlotCount;
}

unsigned long SlabAllocator::getSlotCount() const
{
	return slotCount;
}

unsigned long SlabAllocator::getSlotNo(PhysicalAddress slot) const
{
	return ((unsigned char*)slot - (unsigned char*)mem)/SLOT_SIZE;
}

unsigned long SlabAllocator::getFreeSlotCount() const
{
	return freeSlotCount;
}