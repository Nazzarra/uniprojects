#include "framepool.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::string charToHex(char ch)
{
	constexpr char hexNums[] = {'0', '1', '2', '3', '4', '5', '6', '7',
								'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	std::string rezString = "";
	rezString += hexNums[(ch & 0xF0) >> 4];
	rezString += hexNums[ch & 0x0F];
	return rezString;
}

FramePool::FramePool(PhysicalAddress memory, PageNum memorySize)
	: frameAllocator(memory, memorySize), pFreeFramesHead(nullptr)
{
	frames = new FrameDescr[memorySize];
	for (int i = 0; i < memorySize; ++i) {
		frames[i].frameStart = frameAllocator.getSlot();
		frames[i].pNextFreeFrame = pFreeFramesHead;
		pFreeFramesHead = frames + i;
	}

	std::ofstream dump("dump\\frame_init.txt");
	dump << "Base mem: " << memory << std::endl;
	for (int i = 0; i < memorySize; ++i)
		dump << "Gotten memory: " << frames[i].frameStart << " Expected memory: " << (void*)((unsigned char*)memory + PAGE_SIZE * i) << std::endl;
	
}

FramePool::~FramePool()
{
	delete[] frames;
}

FrameDescr* FramePool::getFreeFrame()
{
	FrameDescr* frame = nullptr;
	if (pFreeFramesHead) {
		frame = pFreeFramesHead;
		frame->setValid(1);
		pFreeFramesHead = pFreeFramesHead->pNextFreeFrame;
	}
	return frame;
}

void FramePool::freeFrame(FrameDescr* frame)
{
	frame->pNextFreeFrame = pFreeFramesHead;
	frame->setValid(0);
	pFreeFramesHead = frame;
}

bool FramePool::hasFreeFrames()
{
	return pFreeFramesHead != nullptr;
}

void FramePool::dumpFrameMemory()
{
	std::stringstream converter;
	converter << "dump\\frame_dump_" << dumpCnt++ << ".txt";
	std::ofstream dumpFile(converter.str());

	for (int i = 0; i < frameAllocator.getSlotCount(); ++i) {
		dumpFile << i << ".: ";
		if (frames[i].getValid())
			dumpFile << "USED";
		else
			dumpFile << "------";
		dumpFile << std::endl;
	}
	
	dumpFile << "Begin memory dump:" << std::endl;
	for (int i = 0; i < frameAllocator.getSlotCount(); ++i) {
		dumpFile << i << ".: ";
		if (!frames[i].getValid()) {
			dumpFile << "--------------------------------" << std::endl;
		}
		else {
			dumpFile << " USED" << std::endl;
			char *mem = (char*)frames[i].frameStart;
			for (int i = 0; i < PAGE_SIZE; ++i) {
				dumpFile << charToHex(mem[i]) << " ";
				if ((i + 1) % 32 == 0)
					dumpFile << std::endl;
			}
		}
	}
	dumpFile << "End memory dump";
}