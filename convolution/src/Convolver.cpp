#include <cstdio>
#include <iostream>
#include <math.h>

#include "Convolver.h"
#include "Wave.h"

#define SWAP(a,b)  tempr=(a);(a)=(b);(b)=tempr

using namespace std;

/* Written By Leonard Manzara. Modified by Chris Nguyen. */
void Convolver::convolve(const float x[], int N, const float h[], int M, float y[], int P)
{
	int n, m;

	/*  Make sure the output buffer is the right size: P = N + M - 1  */
	if (P != (N + M - 1)) {
		printf("Output signal vector is the wrong size\n");
		printf("It is %-d, but should be %-d\n", P, (N + M - 1));
		printf("Aborting convolution\n");
		return;
	}

	/*  Clear the output buffer y[] to all zero values  */  
	for (n = 0; n < P; n++)
		y[n] = 0;

	/*  Do the convolution  */
	/*  Outer loop:  process each input value x[n] in turn  */
	for (n = 0; n < N; n++) {
		/*  Inner loop:  process x[n] with each sample of h[]  */
		for (m = 0; m < M; m++) {
			y[n+m] += (float) (x[n] * h[m]);	
		}
	}
}

/* Written By Abbas Sarraf. Modified by Chris Nguyen. */
void Convolver::convolve(SoundFile* dry, SoundFile* ir, short y[], int P)
{
	float *resultTemp = new float[P];

	// TODO: Combine these two loops
	// TODO: Replace 1.0 with 1.0f here and a further down
	

	// TODO: Move pow() out of loop, don't need to recalculate every iteration
	// TODO: Replace pow with either adding every loop or bit shifting and adding
	//TODO: Replace 2.0 with 2.0f
	float *x = new float[dry->getDataSize()];
	float inputSignalMax = dry->getData()[0];
	for (int i = 0; i < dry->getDataSize(); i++) {
		x[i] = (float) dry->getData()[i] / pow(2.0, dry->getBitsPerSample() - 1);

		if (dry->getData()[i] > inputSignalMax)
			inputSignalMax = dry->getData()[i];
	}


	if (ir->getNumChannels() == 1) {
		float *h = new float[ir->getDataSize()];
		for (int i = 0; i < ir->getDataSize(); i++) {
			h[i] = (float) ir->getData()[i] / pow(2.0, ir->getBitsPerSample() - 1);
		}

		convolve(x, dry->getDataSize(), h, ir->getDataSize(), resultTemp, P);
		delete[] h;
	}
	else {//if (ir->getNumChannels() == 2) {
		// TODO: Calculate ir->getDataSize()/2 once, and use bitshift
		/* Split stereo impulse response into left and right channels */
		short* irLeft = new short[ir->getDataSize()/2];
		short* irRight = new short[ir->getDataSize()/2];
		ir->splitChannels(irLeft, irRight, ir->getDataSize()/2);

		/* Normalize both left and right channels between -1 and 1 */
		float* hLeft = new float[ir->getDataSize()/2];
		float* hRight = new float[ir->getDataSize()/2];
		for (int i = 0; i < ir->getDataSize()/2; i++) {
			hLeft[i] = (float) irLeft[i] / pow(2.0, ir->getBitsPerSample() - 1);
			hRight[i] = (float) irRight[i] / pow(2.0, ir->getBitsPerSample() - 1);
		}
		
		/* Convolve mono dry recording with both left and right IR channels, separately */
		float* resultTempLeft = new float[P/2];
		float* resultTempRight = new float[P/2];
		convolve(x, dry->getDataSize(), hLeft, ir->getDataSize()/2, resultTempLeft, P/2);
		convolve(x, dry->getDataSize(), hRight, ir->getDataSize()/2, resultTempRight, P/2);
		
		/* Interleave the left and right convolutions */
		ir->interleave(resultTempLeft, resultTempRight, P/2, resultTemp);

		delete[] irLeft;
		delete[] irRight;
		delete[] hLeft;
		delete[] hRight;
		delete[] resultTempLeft;
		delete[] resultTempRight;
	}

	/* Find the lower and upper bounds of the convolved output*/
	float oldMin = resultTemp[0], oldMax = resultTemp[0];
	for (int i = 0; i < P; i++) {
		if (resultTemp[i] < oldMin)
			oldMin = resultTemp[i];
		if (resultTemp[i] > oldMax)
			oldMax = resultTemp[i];
	}

	/* Normalize bounds of convolved output back to between -1 and 1 */
	// TODO: Replace 1.0 with 1.0f
	// TODO: Merge below two loops
	for (int i = 0; i < P; i++) {
		resultTemp[i] = normalize(resultTemp[i], oldMin, oldMax, -1.0, 1.0);
	}

	for (int i = 0; i < P; i++)	{
		y[i] = symmetricalRound(resultTemp[i] * inputSignalMax);//(pow(2.0, 16-1) - 1));
	}

	delete[] x;
	delete[] resultTemp;
}

//  The four1 FFT from Numerical Recipes in C,
//  p. 507 - 508.
//  Note:  changed float data types to double.
//  nn must be a power of 2, and use +1 for
//  isign for an FFT, and -1 for the Inverse FFT.
//  The data is complex, so the array size must be
//  nn*2. This code assumes the array starts
//  at index 1, not 0, so subtract 1 when
//  calling the routine (see main() below).

void Convolver::four1(double data[], int nn, int isign)
{
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    n = nn << 1;
    j = 1;

    for (i = 1; i < n; i += 2) {
	if (j > i) {
	    SWAP(data[j], data[i]);
	    SWAP(data[j+1], data[i+1]);
	}
	m = nn;
	while (m >= 2 && j > m) {
	    j -= m;
	    m >>= 1;
	}
	j += m;
    }

    mmax = 2;
    while (n > mmax) {
	istep = mmax << 1;
	theta = isign * (6.28318530717959 / mmax);
	wtemp = sin(0.5 * theta);
	wpr = -2.0 * wtemp * wtemp;
	wpi = sin(theta);
	wr = 1.0;
	wi = 0.0;
	for (m = 1; m < mmax; m += 2) {
	    for (i = m; i <= n; i += istep) {
		j = i + mmax;
		tempr = wr * data[j] - wi * data[j+1];
		tempi = wr * data[j+1] + wi * data[j];
		data[j] = data[i] - tempr;
		data[j+1] = data[i+1] - tempi;
		data[i] += tempr;
		data[i+1] += tempi;
	    }
	    wr = (wtemp = wr) * wpr - wi * wpi + wr;
	    wi = wi * wpr + wtemp * wpi + wi;
	}
	mmax = istep;
    }
}

// Written by Abbas Sarraf.
// signal: your actual data
// signaleStructured will be set after this function is called
void Convolver::timeDomainToFreqDomain(short *signal, int signalSize, double *signaleStructured, int structuredSize)
{		
	
	int i;
	for ( i = 0; i < signalSize; i++ )
	{
		signaleStructured[i*2] 		= signal[i]; 	// the real part
		signaleStructured[i*2+1]	= 0;				// the img part
	}
	
	for (; i < structuredSize; i++ ) // this was a bug in the previous code, now it is fixed
	{
		signaleStructured[2*i] = 0;
		signaleStructured[2*i+1] = 0;
	}
	
	four1(signaleStructured-1, structuredSize, 1);
	
}

// Written by Abbas Sarraf.
// size indicates how many complex numbers we have in f, g, and r
// i.e size = actual size of f divided by 2
void Convolver::fftConvolve( double *f, double *g, double *r, int size )
{
	for ( int i = 0; i < size * 2; i+=2 )
	{
		// do the complex number multiplication
		// for more on complex numbers visit http://www.clarku.edu/~djoyce/complex/mult.html
		float real	= f[i] * g[i] - f[i+1] * g[i+1];
		float img	= f[i] * g[i+1] + f[i+1] * g[i];
		r[i] 		= real;
		r[i+1] 		= img;
	}
}

// TODO: Write in assembly or macro it
short Convolver::symmetricalRound(float value)
{
	if (value >= 0.0f)
		return (short) floor(value + 0.5f);
	else
		return (short) ceil(value - 0.5f);
}

//TODO: Don't need to recalculate all that stuff. Only need to calculate in-fromMin. pass in the rest
float Convolver::normalize(float in, float fromMin, float fromMax, float toMin, float toMax)
{
	return ( (toMax-toMin) * (in-fromMin) / (fromMax-fromMin) ) + toMin;
}