#ifndef TESTCONVOLVER_H
#define TESTCONVOLVER_H

#include "Test.h"
#include "SoundFile.h"

class TestConvolver : Test
{
public:
	static void convolve_EmptySignal();
	static void convolve_BoundarySizes();
	static void convolve_Test();
	static void zeroPadAndTimeToFreqDomain_InvalidSignal();
	static void zeroPadAndTimeToFreqDomain_ValidSignal();
	static void fftConvolve_SmallSize();
	static void fftConvolve_Test();
	static void normalize_Test();
	static void dataToSignal_Test();
	static void signalToData_Test();
	static void complexSignalToData_Test();
	static void symmetricalRound_Test();
	static void runAllTests();
};

#endif