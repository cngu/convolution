#include <cstdio>
#include <iostream>
#include <math.h>

#include "Convolver.h"
#include "Wave.h"

#define SWAP(a,b)  tempr=(a);(a)=(b);(b)=tempr

using namespace std;

/* Written By Leonard Manzara. Modified by Chris Nguyen. */
void Convolver::convolve(const double x[], int N, const double h[], int M, double y[], int P)
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
		y[n] = 0.0;

	/*  Do the convolution  */
	/*  Outer loop:  process each input value x[n] in turn  */
	// TODO: Try to flip the two loops since N probably > M. 
	for (n = 0; n < N; n++) {
		/*  Inner loop:  process x[n] with each sample of h[]  */
		for (m = 0; m < M; m++) {
			y[n+m] += (double) (x[n] * h[m]);	
		}
	}
}

void Convolver::convolve(SoundFile* dry, SoundFile* ir, short y[], int P)
{
	// TODO: Combine these two loops	

	// TODO: Move pow() out of loop, don't need to recalculate every iteration
	// TODO: Replace pow with either adding every loop or bit shifting and adding
	double* resultTemp = new double[P];

	double* x = new double[dry->getDataSize()];
	Convolver::dataToSignal(dry->getData(), dry->getDataSize(), dry->getAbsMinValue(), x);

	double* h = new double[ir->getDataSize()];
	Convolver::dataToSignal(ir->getData(), ir->getDataSize(), ir->getAbsMinValue(), h);
		
	if (ir->getNumChannels() == 1) {
		convolve(x, dry->getDataSize(), h, ir->getDataSize(), resultTemp, P);

		/* Find the lower and upper bounds of the convolved output */
		double oldMin = resultTemp[0], oldMax = resultTemp[0];
		for (int i = 0; i < P; i++) {
			if (resultTemp[i] < oldMin)
				oldMin = resultTemp[i];
			if (resultTemp[i] > oldMax)
				oldMax = resultTemp[i];
		}

		/* Normalize bounds of convolved output back to between -1 and 1 */
		for (int i = 0; i < P; i++) {
			resultTemp[i] = normalize(resultTemp[i], oldMin, oldMax, -1.0, 1.0);
		}

		/* Scale convolved signal back to short integer data */
		Convolver::signalToData(resultTemp, P, dry->getMaxValue(), y);
	}
	else if (ir->getNumChannels() == 2) {
		// TODO: Calculate ir->getDataSize()/2 once, and use bitshift. Constants (use const!!!!) and strength reduction, respectively.

		/* Split stereo impulse response into left and right channels */
		double* hLeft = new double[ir->getDataSize()/2];
		double* hRight = new double[ir->getDataSize()/2];
		
		for (int i = 0; i < ir->getDataSize()/2; i++) {
			hLeft[i] = h[i*2];
			hRight[i] = h[i*2 + 1];
		}

		/* Convolve mono dry recording with both left and right IR channels, separately */
		double* resultTempLeft = new double[P/2];
		double* resultTempRight = new double[P/2];
		convolve(x, dry->getDataSize(), hLeft, ir->getDataSize()/2, resultTempLeft, P/2);
		convolve(x, dry->getDataSize(), hRight, ir->getDataSize()/2, resultTempRight, P/2);
		
		/* Find the lower and upper bounds of the convolved output, separately! */
		// TODO: If desperate, use algorithm's min_element or w/e instead. THat loops 4 times for each left/right min/max.
		// Optimize it by using the below
		double oldLeftMin = resultTempLeft[0], oldLeftMax = resultTempLeft[0];
		double oldRightMin = resultTempRight[0], oldRightMax = resultTempRight[0];
		for (int i = 0; i < P/2; i++) {
			if (resultTempLeft[i] < oldLeftMin)
				oldLeftMin = resultTempLeft[i];
			if (resultTempLeft[i] > oldLeftMax)
				oldLeftMax = resultTempLeft[i];
			if (resultTempRight[i] < oldRightMin)
				oldRightMin = resultTempRight[i];
			if (resultTempRight[i] > oldRightMax)
				oldRightMax = resultTempRight[i];
		}

		/* Normalize bounds of both left and right IR channels back to be between -1 and 1 */
		for (int i = 0; i < P/2; i++) {
			resultTempLeft[i] = normalize(resultTempLeft[i], oldLeftMin, oldLeftMax, -1.0, 1.0);
			resultTempRight[i] = normalize(resultTempRight[i], oldRightMin, oldRightMax, -1.0, 1.0);
		}

		short* yLeft = new short[P/2];
		short* yRight = new short[P/2];

		/* Scale both left and right convolved signals back to short integer data */
		Convolver::signalToData(resultTempLeft, P/2, dry->getMaxValue(), yLeft);
		Convolver::signalToData(resultTempRight, P/2, dry->getMaxValue(), yRight);

		/* Interleave the left and right convolutions */
		for (int i = 0; i < P; i+=2) {
			y[i] = yLeft[i/2];
			y[i+1] = yRight[i/2];
		}

		delete[] hLeft;
		delete[] hRight;
		delete[] resultTempLeft;
		delete[] resultTempRight;
	}

	delete[] x;
	delete[] h;
	delete[] resultTemp;
}

//  The four1 FFT from Numerical Recipes in C,
//  p. 507 - 508.
//  Note:  changed double data types to double.
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

void Convolver::zeroPadAndTimeToFreqDomain(double *timeDomain, int timeDomainLen, double *outputFreqDomain, int structuredSize)
{		
	int i = 0;
	for (; i < timeDomainLen; i++ )	{
		outputFreqDomain[i*2] = timeDomain[i];
		outputFreqDomain[i*2+1] = 0;
	}
	for (; i < structuredSize; i++ ) {
		outputFreqDomain[i*2] = 0;
		outputFreqDomain[i*2+1] = 0;
	}
	four1(outputFreqDomain-1, structuredSize, 1);
}

void Convolver::fftConvolve(double *x, double *h, double *r, int size)
{
	for (int i = 0; i < size*2; i+=2) {
		r[i] = x[i] * h[i] - x[i+1] * h[i+1];
		r[i+1] = x[i] * h[i+1] + x[i+1] * h[i];
	}
}

void Convolver::dataToSignal(const short* data, int len, int min, double* signal)
{
	// TODO: do more than one thing per iteration
	for (int i = 0; i < len; i++) {
		signal[i] = (double) data[i] / min;
	}
}

void Convolver::signalToData(const double* signal, int signalLen, int scale, short* data) 
{
	// TODO: do more than one thing per iteration
	for (int i = 0; i < signalLen; i++) {
		data[i] = Convolver::symmetricalRound(signal[i] * scale);
	}
}

void Convolver::complexSignalToData(const double* signal, int signalLen, int scale, short* data)
{
	// TODO: do more than one thing per iteration
	for (int i = 0; i < signalLen; i+=2) {
		data[i/2] = Convolver::symmetricalRound(signal[i] * scale);
	}
}

//TODO: Don't need to recalculate all that stuff. Only need to calculate in-fromMin. pass in the rest
double Convolver::normalize(double in, double fromMin, double fromMax, double toMin, double toMax)
{
	return ( (toMax-toMin) * (in-fromMin) / (fromMax-fromMin) ) + toMin;
}

// TODO: Write in assembly or macro it
short Convolver::symmetricalRound(double value)
{
	if (value >= 0.0)
		return (short) floor(value + 0.5);
	else
		return (short) ceil(value - 0.5);
}