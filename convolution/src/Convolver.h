#ifndef CONVOLVER_H
#define CONVOLVER_H

#include "SoundFile.h"

class Convolver
{
private:

public:
	static void convolve(const double x[], int N, const double h[], int M, double y[], int P);
	static void convolve(SoundFile* dry, SoundFile* ir, short y[], int P);

	static void four1(double data[], int nn, int isign);
	static void zeroPadAndTimeToFreqDomain(double *timeDomain, int timeDomainLen, double *outputFreqDomain, int structuredSize);
	static void complexMultiplication(double* x, double* h, double* r, int size);
	static void fftConvolve(double x[], int N, double h[], int M, short y[], int P);
	static void fftConvolve(SoundFile* dry, SoundFile* ir, short y[], int P);
	
	static void dataToSignal(const short* data, int len, int min, double* signal);
	static void signalToData(const double* signal, int len, int scale, short* data);
	static void complexSignalToData(const double* signal, int len, int scale, short* data);
	static double normalize(double in, double oldMin, double oldMax, double newMin, double newMax);
	static short symmetricalRound(double value);

private:
	
	
};

#endif