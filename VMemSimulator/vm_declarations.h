#ifndef _VM_DECLARATIONS_H
#define _VM_DECLARATIONS_H

typedef unsigned long PageNum;
typedef unsigned long VirtualAddress;
typedef void* PhysicalAddress;
typedef unsigned long Time;
typedef unsigned long AccessRight;
//enum Status { OK, PAGE_FAULT, TRAP };
enum Status { OK, PAGE_FAULT, TRAP };
enum AccessType { READ, WRITE, READ_WRITE, EXECUTE };
typedef unsigned ProcessId;

typedef unsigned long ClusterNo;

//Page size in bytes
#define PAGE_SIZE 1024

constexpr unsigned long PAGE_NO_OVERFLOW = 1 << 14;

//Virtual address size in bits
constexpr unsigned long VIRTUAL_ADDRESS_SIZE = 24;

//Virtual address offset bits count
constexpr unsigned long OFFSET_BITS = 10;

//Virtual address page bits count
constexpr unsigned long PAGE_BITS = 14;

//PMT is made out of 2 levels

//Bits in virtual address as PMT level one entry
constexpr unsigned long PMT_ONE_BITS = 8;

//Bits in virtual address as PMT level two entry
constexpr unsigned long PMT_TWO_BITS = 6;

//Number of PMT level two tables 
constexpr unsigned long PMT_ONE_TABLE_SIZE = 1 << PMT_ONE_BITS;

//Number of pages per PMT level two table
constexpr unsigned long PMT_TWO_PAGE_COUNT = 1 << PMT_TWO_BITS;

constexpr unsigned long OFFSET_MASK = (1 << OFFSET_BITS) - 1;
constexpr unsigned long PMT_LEVEL_TWO_MASK = (1 << PMT_TWO_BITS) - 1;

inline bool	isAllignedToPageStart(VirtualAddress address)
{
	return (address & OFFSET_MASK) == 0;
}

inline PageNum getVAPage(VirtualAddress vas)
{
	return (vas >> OFFSET_BITS);
}

inline unsigned short getPMTOneEntry(VirtualAddress address)
{
	return address >> (OFFSET_BITS + PMT_TWO_BITS);
}

inline unsigned short getPMTTwoEntry(VirtualAddress address) 
{
	return (address >> OFFSET_BITS) & PMT_LEVEL_TWO_MASK;
}

inline unsigned short getPagePMTOneEntry(PageNum pageNo)
{
	return (unsigned short)(pageNo >> PMT_TWO_BITS);
}

inline unsigned short getPagePMTTwoEntry(PageNum pageNo)
{
	return (unsigned short)(pageNo & PMT_LEVEL_TWO_MASK);
}

inline unsigned char getReadFlag(AccessRight flags)
{
	return flags == READ || flags == READ_WRITE;
}

inline unsigned char getWriteFlag(AccessRight flags)
{
	return flags == WRITE || flags == READ_WRITE;
}

inline unsigned char getExecuteFlag(AccessRight flags)
{
	return flags == EXECUTE;
}

inline VirtualAddress makeAddress(PageNum page, unsigned long offset)
{
	return (VirtualAddress)((page << OFFSET_BITS) | offset);
}

#endif