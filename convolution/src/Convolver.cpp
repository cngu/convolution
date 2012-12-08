#include <cstdio>
#include <iostream>
#include <math.h>

#include "Convolver.h"
#include "Wave.h"

#define SWAP(a,b)  tempr=(a);(a)=(b);(b)=tempr

using namespace std;

/* Written By Leonard Manzara. Modified by Chris Nguyen. */
void Convolver::multiplicationConvolution(const double x[], int N, const double h[], int M, double y[], int P)
{
	int n, m;

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

void Convolver::convolve(double x[], int N, double h[], int M, short y[], int P)
{
	/*  Make sure the output buffer is the right size: P = N + M - 1  */
	if (N == 0 || M == 0 || P != (N + M - 1)) {
		return;
	}

	/* Perform Time domain convolution */
	double* resultTemp = new double[P];
	multiplicationConvolution(x, N, h, M, resultTemp, P);

	// TODO: First use min_element, then use this AND combine these two loops	
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
	Convolver::signalToData(resultTemp, P, 32767, y);
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

void Convolver::complexMultiplicationConvolution(double* x, double* h, double* r, int size)
{
	for (int i = 0; i < size*2; i+=2) {
		r[i] = x[i] * h[i] - x[i+1] * h[i+1];
		r[i+1] = x[i] * h[i+1] + x[i+1] * h[i];
	}
}

void Convolver::fftConvolve(double x[], int N, double h[], int M, short y[], int P)
{
	/* Find suitable zero-pad length to prevent circular convolution */
	int structuredSize = 1;
	while (structuredSize < P)
		structuredSize *= 2;	// TODO: Replace with bit shift

	double* X = new double[structuredSize * 2];
	double* H = new double[structuredSize * 2];

	/* Zero-pad normalized signals and convert to Frequency Domain */	
	Convolver::zeroPadAndTimeToFreqDomain(x, N, X, structuredSize);
	Convolver::zeroPadAndTimeToFreqDomain(h, M, H, structuredSize);

	/* Perform Frequency-Domain Convolution */
	double* R = new double[structuredSize * 2];
	Convolver::complexMultiplicationConvolution(X, H, R, structuredSize);

	/* Convert Frequency-Domain back to Time-Domain */
	Convolver::four1(R-1, structuredSize, -1);

	/* Divide everything by N and find min/max */
	// TODO: Use min_element or w/e and then optimize it to this current version
	double min = R[0]/(double)structuredSize, 
		   max = R[0]/(double)structuredSize;

	for (int i = 0; i < structuredSize*2; i+=2) {
		R[i] /= (double)structuredSize;

		if (R[i] < min)
			min = R[i];
		if (R[i] > max)
			max = R[i];
	}
	
	// TODO: Merge with loop above
	// TODO: Could also partial unroll it, be careful
	for (int i = 0; i < structuredSize*2; i+=2) {
		R[i] = Convolver::normalize(R[i], min, max, -1.0, 1.0);
	}

	/* Scale result back up to short */
	Convolver::complexSignalToData(R, P*2, 32767, y);

	delete[] X;
	delete[] H;
	delete[] R;
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
//TODO: OR just macro it (put it in header Convolver.h)
double Convolver::normalize(double in, double fromMin, double fromMax, double toMin, double toMax)
{
	return ( (toMax-toMin) * (in-fromMin) / (fromMax-fromMin) ) + toMin;
}

// TODO: Write in assembly or macro it (put it in header Convolver.h)
short Convolver::symmetricalRound(double value)
{
	if (value >= 0.0)
		return (short) floor(value + 0.5);
	else
		return (short) ceil(value - 0.5);
}