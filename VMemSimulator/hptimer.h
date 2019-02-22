#pragma once

#include "windows.h"
class PerformanceCalculator
{
	LARGE_INTEGER startCounter;
	LARGE_INTEGER stopCounter;
	LARGE_INTEGER frequency;

public:
	PerformanceCalculator()
	{
		startCounter.QuadPart = stopCounter.QuadPart = 0;
		QueryPerformanceFrequency(&frequency);
	}

	void start()
	{
		QueryPerformanceCounter(&startCounter);
	}

	void stop()
	{
		QueryPerformanceCounter(&stopCounter);
	}

	double elapsedMillis() const
	{
		return(stopCounter.QuadPart - startCounter.QuadPart)*1000. / frequency.QuadPart;
	}

	double elapsedSeconds() const
	{
		return (stopCounter.QuadPart - startCounter.QuadPart)*1. / frequency.QuadPart;
	}

	long long elaspedMicrosecs() const
	{
		return (stopCounter.QuadPart - startCounter.QuadPart)*1000000 / frequency.QuadPart;
	}
};
