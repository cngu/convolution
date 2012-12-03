#ifndef CONVOLVER_H
#define CONVOLVER_H

#include "SoundFile.h"

class Convolver
{
private:

public:
	static void convolve(const float x[], int N, const float h[], int M, float y[], int P);
	static void convolve(SoundFile* dry, SoundFile* ir, short y[], int P);

	static void four1(double data[], int nn, int isign);
	static void timeDomainToFreqDomain(short *signal, int signalSize, double *signaleStructured, int structuredSize);
	static void fftConvolve(double *f, double *g, double *r, int size);
	
	static float normalize(float in, float oldMin, float oldMax, float newMin, float newMax);

private:
	static short symmetricalRound(float value);
	
};

#endif