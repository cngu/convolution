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
	static void fftConvolve(double *x, double *h, double *r, int size);
	
	static double normalize(double in, double oldMin, double oldMax, double newMin, double newMax);
	static short symmetricalRound(double value);

private:
	
	
};

#endif