#ifndef CONVOLVE_H
#define CONVOLVE_H

// TODO: Rename to SignalProcessor? Handles FFT DFT too?
class Convolve 
{
private:
	// Variables

public:
	void convolve();
	void convolve(const float x[], int N, const float h[], int M, float y[], int P);

protected:

};

#endif