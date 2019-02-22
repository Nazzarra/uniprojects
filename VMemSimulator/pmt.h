#ifndef _VM_PMT_DEFS_H
#define _VM_PMT_DEFS_H

#include "vm_declarations.h"
#include <cstring>
#include <string>
#include <sstream>
#include <new>
#include <iostream>

struct PmtLevelTwo;
struct FrameDescr;
struct SharedSegmentKernel;

constexpr unsigned char FL_VALID_SHIFT = 1;
constexpr unsigned char FL_X_SHIFT = 2;
constexpr unsigned char FL_W_SHIFT = 3;
constexpr unsigned char FL_R_SHIFT = 4;
constexpr unsigned char FL_DIRTY_SHIFT = 5;
constexpr unsigned char FL_ROOT_SHIFT = 6;

constexpr unsigned char FL_REF_MASK = 1;
constexpr unsigned char FL_VALID_MASK = 2;
constexpr unsigned char FL_X_MASK = 4;
constexpr unsigned char FL_W_MASK = 8;
constexpr unsigned char FL_R_MASK = 16;
constexpr unsigned char FL_DIRTY_MASK = 32;
constexpr unsigned char FL_ROOT_MASK = 64;

constexpr unsigned char CFL_X_SHIFT = 1;
constexpr unsigned char CFL_W_SHIFT = 2;
constexpr unsigned char CFL_R_SHIFT = 3;

constexpr unsigned char CFL_COW_MASK = 1;
constexpr unsigned char CFL_X_MASK = 2;
constexpr unsigned char CFL_W_MASK = 4;
constexpr unsigned char CFL_R_MASK = 8;

constexpr unsigned short COW_INVALID_PID = (unsigned short)(~(0U));

struct PmtLevelOne
{
	PmtLevelTwo* pPmtTable[PMT_ONE_TABLE_SIZE];
};

template <typename T>
static std::string convertNumToBinary(T num)
{
	std::stringstream converter;
	unsigned long mask = 1LU << (sizeof(T) * 8 - 1);
	while (mask) {
		if (num & mask)
			converter << "1";
		else
			converter << "0";

		mask >>= 1;
	}
	return converter.str();
}

struct PageDescriptor
{
	ClusterNo cluster;
	FrameDescr* frame;
	union {
		PageDescriptor* rootDescr;
		struct {
			unsigned short prevProcPid; //PID of the prev process sharing this page
			unsigned short nextProcPid; //PID of the next process sharing this page 
		} cowList; //Gotta make it standard
	};
	unsigned short history;
	unsigned char flags; // isRoot, dirty, r, w, x, valid, referenced
	unsigned char flagsCoW; // r, w, x, IsCoW

	PageDescriptor()
		: cluster(~(0LU)), frame(nullptr), rootDescr(nullptr), history(0), flags(0), flagsCoW(0)
	{
	}

	void setRead(unsigned char bit)
	{
		flags = (flags & ~FL_R_MASK) | (bit << FL_R_SHIFT);
	}

	void setWrite(unsigned char bit)
	{
		flags = (flags & ~FL_W_MASK) | (bit << FL_W_SHIFT);
	}

	void setExecute(unsigned char bit)
	{
		flags = (flags & ~FL_X_MASK) | (bit << FL_X_SHIFT);
	}

	void setValid(unsigned char bit)
	{
		flags = (flags & ~FL_VALID_MASK) | (bit << FL_VALID_SHIFT);
	}

	void setReferenced(unsigned char bit)
	{
		flags = (flags & ~FL_REF_MASK) | bit;
	}

	void setDirty(unsigned char dirty)
	{
		flags = (flags & ~FL_DIRTY_MASK) | (dirty << FL_DIRTY_SHIFT);
	}

	void setRoot(unsigned char root)
	{
		flags = (flags & ~FL_ROOT_MASK) | (root << FL_ROOT_SHIFT);
	}

	unsigned char getRead()
	{
		return (flags & FL_R_MASK) >> FL_R_SHIFT;
	}

	unsigned char getWrite()
	{
		return (flags & FL_W_MASK) >> FL_W_SHIFT;
	}

	unsigned char getExecute()
	{
		return (flags & FL_X_MASK) >> FL_X_SHIFT;
	}

	unsigned char getValid()
	{
		return (flags & FL_VALID_MASK) >> FL_VALID_SHIFT;
	}

	unsigned char getReferenced()
	{
		return flags & FL_REF_MASK;
	}

	unsigned char getDirty()
	{
		return (flags & FL_DIRTY_MASK) >> FL_DIRTY_SHIFT;
	}

	unsigned char getRoot()
	{
		return (flags & FL_ROOT_MASK) >> FL_ROOT_SHIFT;
	}

	unsigned char isCoW()
	{
		return flagsCoW & CFL_COW_MASK;
	}

	unsigned char isShared()
	{
		return !isCoW() && rootDescr;
	}
	
	void convertToCoW()
	{
		flagsCoW = CFL_COW_MASK;
		if (getRead()) {
			flagsCoW |= CFL_R_MASK;
		}
		if (getWrite()) {
			flagsCoW |= CFL_W_MASK;
			setWrite(0);
		}
		cowList.nextProcPid = cowList.prevProcPid = COW_INVALID_PID;
	}

	void restoreFlagsPreCoW()
	{
		if (flagsCoW & CFL_W_MASK)
			setWrite(1);
		rootDescr = nullptr;
		flagsCoW = 0;
	}

	unsigned char isNextPidValid() 
	{
		return cowList.nextProcPid != COW_INVALID_PID;
	}

	unsigned char isPrevPidValid()
	{
		return cowList.prevProcPid != COW_INVALID_PID;
	}

	bool isValidCluster()
	{
		return cluster != ~(0LU);
	}

	void print(std::ostream& out) const {
		out << cluster << " " << frame << " " << rootDescr << " " << std::hex << convertNumToBinary(flags) << " " << convertNumToBinary(flagsCoW) << std::dec << std::endl;
	}
};

struct PmtLevelTwo
{
	PageDescriptor pageDescr[PMT_TWO_PAGE_COUNT];

	void init() {
		for (unsigned short i = 0; i < PMT_TWO_PAGE_COUNT; ++i) {
			pageDescr[i].cluster = ~(0LU);
			pageDescr[i].frame = nullptr;
			pageDescr[i].rootDescr = nullptr;

			pageDescr[i].history = 0;
			pageDescr[i].flags = 0;
			pageDescr[i].flagsCoW = 0;
		}
	}

	bool hasValidPages() {
		for (unsigned short i = 0; i < PMT_TWO_PAGE_COUNT; ++i)
			if (pageDescr[i].getValid())
				return true;

		return false;
	}
};

constexpr unsigned SHARED_SEGMENT_CLUSTERS_PER_DUMP = PAGE_SIZE / sizeof(ClusterNo);

struct SharedSegmentDiskDump
{
	ClusterNo pageCluster[SHARED_SEGMENT_CLUSTERS_PER_DUMP];
};

#endif