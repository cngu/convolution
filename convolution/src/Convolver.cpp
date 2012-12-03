#include <cstdio>
#include <iostream>
#include <math.h>
#include <memory>

#include "Convolver.h"
#include "Wave.h"

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
void Convolver::convolve(const unique_ptr<Wave>& dry, const unique_ptr<Wave>& ir, short y[], int P)
{
	// TODO: Combine these two loops
	float *x = new float[dry->dataSize];
	cout << "Dry Recording DataSize: " << dry->dataSize << endl;

	// TODO: Move pow() out of loop, don't need to recalculate every iteration
	// TODO: Replace pow with either adding every loop or bit shifting and adding
	//TODO: Replace 2.0 with 2.0f
	for (int i = 0; i < dry->dataSize; i++) {
		x[i] = (float) dry->data[i] / pow(2.0, dry->bitsPerSample - 1);
	}
	cout << "Done normalizing dry recording" << endl;

	cout << "IR DataSize: " << ir->dataSize << endl;
	float *h = new float[ir->dataSize];
	for (int i = 0; i < ir->dataSize; i++) {
		h[i] = (float) ir->data[i] / pow(2.0, ir->bitsPerSample - 1);
	}
	cout << "Done normalizing impulse response" << endl;

	float *yTemp = new float[P];
	convolve(x, dry->dataSize, h, ir->dataSize, yTemp, P);

	/* Find the lower and upper bounds of the convolved output*/
	float oldMin = 1.0f, oldMax = -1.0f;
	float *yTemp2 = new float[P];
	for (int i = 0; i < P; i++) {
		if (yTemp[i] < oldMin)
			oldMin = yTemp[i];
		else if (yTemp[i] > oldMax)
			oldMax = yTemp[i];
	}

	/* Normalize bounds of convolved output back to between -1 and 1 */
	// TODO: Replace 1.0 with 1.0f
	for (int i = 0; i < P; i++) {
		yTemp[i] = normalize(yTemp[i], oldMin, oldMax, -1.0, 1.0);
	}

	for (int i = 0; i < P; i++)	{
		// Output is fixed at 16 bits per sample as per assignment specifications
		y[i] = symmetricalRound(yTemp[i] * (pow(2.0, 16-1) - 1));
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

//TODO: Just pass in similar stuff
float Convolver::normalize(float in, float fromMin, float fromMax, float toMin, float toMax)
{
	return ( (toMax-toMin) * (in-fromMin) / (fromMax-fromMin) ) + toMin;
}