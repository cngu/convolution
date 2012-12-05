#include "TestConvolver.h"
#include "SoundFile.h"

// static void convolve(const double x[], int N, const double h[], int M, double y[], int P);
void TestConvolver::convolve_EmptySignal()
{

}

void TestConvolver::convolve_BoundarySizes()
{

}

void TestConvolver::convolve_Test() 
{

}

void TestConvolver::convolve_NullFilesTest() 
{

}

// static void convolve(SoundFile* dry, SoundFile* ir, short y[], int P);
void TestConvolver::convolve_FilesTest()
{
	// Result: Convolve iR with same IR
	// Expected: load data of both files, normalize both data, convolve them using top convolve above
}

// static void four1(double data[], int nn, int isign);
// Tested indirectly in TestMain.cpp

// static void zeroPadAndTimeToFreqDomain(double *timeDomain, int timeDomainLen, double *outputFreqDomain, int structuredSize);
void TestConvolver::zeroPadAndTimeToFreqDomain_InvalidSignal() // less than -1 or above 1
{

}

void TestConvolver::zeroPadAndTimeToFreqDomain_ValidSignal()
{

}

// static void fftConvolve(double *x, double *h, double *r, int size);
void TestConvolver::fftConvolve_SmallSize()
{

}

void TestConvolver::fftConvolve_Test()
{

}

// static double normalize(double in, double oldMin, double oldMax, double newMin, double newMax);
void TestConvolver::normalize_Test()
{

}

// static short symmetricalRound(double value);
void TestConvolver::symmetricalRound_Test()
{

}

void TestConvolver::runAllTests()
{
	convolve_EmptySignal();
	convolve_BoundarySizes();
	convolve_Test();
	convolve_NullFilesTest();
	convolve_FilesTest();
	zeroPadAndTimeToFreqDomain_InvalidSignal();
	zeroPadAndTimeToFreqDomain_ValidSignal();
	fftConvolve_SmallSize();
	fftConvolve_Test();
	normalize_Test();
	symmetricalRound_Test();
}