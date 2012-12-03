#ifndef CONVOLVER_H
#define CONVOLVER_H

#include <memory>

#include "Wave.h"

class Convolver
{
private:
	// Variables

public:
	static void convolve(const float x[], int N, const float h[], int M, float y[], int P);
	static void convolve(const std::unique_ptr<Wave>& dry, const std::unique_ptr<Wave>& ir, 
                  short y[], int P);

private:
	static short symmetricalRound(float value);
	static float normalize(float in, float oldMin, float oldMax, float newMin, float newMax);
};

#endif