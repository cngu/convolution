#include "TestConvolver.h"
#include "SoundFile.h"
#include "Convolver.h"

#include <iostream>
#include <string>

using namespace std;

// static void convolve(double x[], int N, double h[], int M, short y[], int P);
void TestConvolver::convolve_EmptySignal()
{
	int n = 0, m = 2, p = n+m-1;
	double*x = new double[0];		// empty
	double h[] = {0.2, 0.5};
	short* r = new short[p];
	Test::clearArray(r, p);

	Convolver::convolve(x, n, h, m, r, p);

	short expected[] = {0};
	if (! Test::equalArrays(r, expected, n))
		fail("TestConvolver", string("convolve_EmptySignal"));

	delete[] x;
	delete[] r;
}

void TestConvolver::convolve_BoundarySizes()
{
	int n = 10, m = 2, p = n+m-1;
	double* x = new double[n-1];		// purposely wrong sizes, should be n and m
	double* h = new double[m-2];
	short* r = new short[p];			// should contain convolution of first n-1 of DR and first m-2 points of IR
	Test::clearArray(r, p);

	Convolver::convolve(x, n-1, h, m-2, r, p);

	short expected[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	if (! Test::equalArrays(r, expected, n-1))
		fail("TestConvolver", string("convolve_BoundarySizes"));

	delete[] x;
	delete[] h;
	delete[] r;
}

void TestConvolver::convolve_Test() 
{
	int n = 10, m = 2, p = n+m-1;
	double x[] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
	double h[] = {0.2, 0.5};
	short* r = new short[p];
	Test::clearArray(r, p);

	Convolver::convolve(x, n, h, m, r, p);

	short expected[] = {-32767, -30507, -22598, -14689, -6779, 1130, 9039, 16948, 24858, 32767, 18078};
	if (! Test::equalArrays(r, expected, n))
		fail("TestConvolver", string("convolve_Test"));

	delete[] r;
}

// static void four1(double data[], int nn, int isign);
// Tested indirectly in TestMain.cpp

// static void zeroPadAndTimeToFreqDomain(double *timeDomain, int timeDomainLen, double *outputFreqDomain, int structuredSize);
void TestConvolver::zeroPadAndTimeToFreqDomain_InvalidSignal()
{
	int n = 5, m = 2, p = n+m-1;
	double x[] = {-2, -3, -4, 3, 2};		// invalid signal, less than -1 or above 1

	int structuredSize = 1;
	while (structuredSize < p)
		structuredSize = structuredSize << 2;
	double* r = new double[structuredSize*2];
	Test::clearArray(r, structuredSize*2);

	Convolver::zeroPadAndTimeToFreqDomain(x, n, r, structuredSize);

	double expected[] = {-4, 0, -6.45202, 0.795161, -8.24264, -4, -3.09126, -8.74812, 4, -6, 4.74812, 0.908738, 
						0.242641, 4, -3.20484, 2.45202, -4, 0, -3.20484, -2.45202, 0.242641, -4, 4.74812, -0.908738,
						4, 6, -3.09126, 8.74812, -8.24264, 4, -6.45202, -0.795161};

	if (! Test::equalArrays(r, expected, n))
		fail("TestConvolver", string("zeroPadAndTimeToFreqDomain_InvalidSignal"));

	delete[] r;
}

void TestConvolver::zeroPadAndTimeToFreqDomain_ValidSignal()
{
	int n = 5, m = 2, p = n+m-1;
	double x[] = {-0.2, -0.1, 0.0, 0.1, 0.2};	// valid signal, all between -1 and +1

	int structuredSize = 1;
	while (structuredSize < p)
		structuredSize = structuredSize << 2;
	double* r = new double[structuredSize*2];
	Test::clearArray(r, structuredSize*2);

	Convolver::zeroPadAndTimeToFreqDomain(x, n, r, structuredSize);

	// I'm making smaller inputs from now on...I deserve a cookie for hand calculating this
	double expected[] = {0, 0, -0.25412, 0.25412, -0.541421, -1.249e-016, -0.330656, -0.330656, 1.55431e-016, -0.2, 
						-0.0693437, 0.069343, 7, -0.258579, 2.22045e-016, -0.14588, -0.14588, 0, 0, -0.14588,
						0.14588, -0.258579, 1.249e-016, -0.0693437, -0.0693437, -1.55431e-016, 0.2, -0.330656, 0.330656,
						-0.541421, -2.22045e-016, -0.25412, -0.25412};

	if (! Test::equalArrays(r, expected, n))
		fail("TestConvolver", string("zeroPadAndTimeToFreqDomain_ValidSignal"));

	delete[] r;
}

// static void fftConvolve(double *x, double *h, double *r, int size);
void TestConvolver::fftConvolve_SmallSize()
{
	int n = 10, m = 2, p = n+m-1;
	double x[] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
	double h[] = {0.2, 0.5};
	short* r = new short[p-2];	// Small size, should be p
	Test::clearArray(r, p-2);

	Convolver::convolve(x, n, h, m, r, p-2);

	// Since we used p-2, no convolutions occurred in first p-2 (p-2=9). Thus the first 9 0's in the expected.
	short expected[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, -515, -515};	
	if (! Test::equalArrays(r, expected, p-2))
		fail("TestConvolver", string("fftConvolve_SmallSize"));

	delete[] r;
}

void TestConvolver::fftConvolve_Test()
{
	// Exact same test as TestConvolver::convolve() except this one calls fftConvolve

	int n = 10, m = 2, p = n+m-1;
	double x[] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
	double h[] = {0.2, 0.5};
	short* r = new short[p];
	Test::clearArray(r, p);

	Convolver::fftConvolve(x, n, h, m, r, p);

	short expected[] = {-32767, -30507, -22598, -14689, -6779, 1130, 9039, 16948, 24858, 32767, 18078};
	if (! Test::equalArrays(r, expected, n))
		fail("TestConvolver", string("fftConvolve_Test"));

	delete[] r;
}

// static void dataToSignal(const short* data, int len, int min, double* signal);
void TestConvolver::dataToSignal_Test()
{
	int n = 5; 
	short data[] = {100, 200, 300, 400, -500};
	int len = 5;
	double signal[5];
	Test::clearArray(signal, 5);

	Convolver::dataToSignal(data, 5, 32768, signal);

	double expected[] = {0.00305176, 0.00610352, 0.00915527, 0.012207, -0.0152588};
	if (! Test::equalArrays(signal, expected, 5))
		fail("TestConvolver", string("dataToSignal_Test"));
}

// static void signalToData(const double* signal, int len, int scale, short* data);
void TestConvolver::signalToData_Test()
{
	double signal[] = {0.00305176, 0.00610352, 0.00915527, 0.012207, -0.0152588};
	short expected[] = {100, 200, 300, 400, -500};
	short data[5];
	Test::clearArray(data, 5);

	Convolver::signalToData(signal, 5, 32767, data);

	if (! Test::equalArrays(data, expected, 5))
		fail("TestConvolver", string("signalToData_Test"));
}

// static void complexSignalToData(const double* signal, int len, int scale, short* data);
void TestConvolver::complexSignalToData_Test()
{
	double complex_signal[] = {0.5, 0, 0.8, 0, -0.2, 0};
	short expected[] = {16384, 26214, -6553, 0, 0, 0};
	short data[6];
	Test::clearArray(data, 6);
	
	Convolver::complexSignalToData(complex_signal, 6, 32767, data);
	if (! Test::equalArrays(data, expected, 6))
		fail("TestConvolver", string("complexSignalToData_Test"));
}

// static double normalize(double in, double oldMin, double oldMax, double newMin, double newMax);
void TestConvolver::normalize_Test()
{
	double value = 12345;
	double result = NORMALIZE(value, -32768, 32767, 0, 100);
	double expected = 68.838;
	if (! Test::equalDoubles(result, expected))
		fail("TestConvolver", string("normalize_Test"));
}

// static short symmetricalRound(double value);
void TestConvolver::symmetricalRound_Test()
{
	double v1 = 0.6; double expectedV1 = 1.0;
	double v2 = -0.8; double expectedV2 = -1.0;
	double v3 = 0.5; double expectedV3 = 1.0;

	if ( !Test::equalDoubles(Convolver::symmetricalRound(v1), expectedV1) ||
		 !Test::equalDoubles(Convolver::symmetricalRound(v2), expectedV2) ||
		 !Test::equalDoubles(Convolver::symmetricalRound(v3), expectedV3))
		fail("TestConvolver", string("symmetricalRound_Test"));
}

void TestConvolver::runAllTests()
{
	cout << "Running TestConvolver..." << endl;
	convolve_EmptySignal();
	convolve_BoundarySizes();
	convolve_Test();
	zeroPadAndTimeToFreqDomain_InvalidSignal();
	zeroPadAndTimeToFreqDomain_ValidSignal();
	fftConvolve_SmallSize();
	fftConvolve_Test();
	dataToSignal_Test();
	signalToData_Test();
	complexSignalToData_Test();
	normalize_Test();
	symmetricalRound_Test();
	cout << "TestConvolver completed.\n" << endl;
}