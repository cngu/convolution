#include <iostream>
#include <string>

#include "RegressionTest.h"
#include "TestConvolver.h"

#include "Convolver.h"
#include "SoundFile.h"
#include "Wave.h"
#include "Snd.h"
#include "Aiff.h"

// Control directives
#define FFT		// Uncomment to use Frequency-Domain Convolution rather than Time Domain
//#define TESTS		// Uncomment to run tests

using namespace std;

void usage()
{
	cout << "Usage: convolve inputfile IRfile outputfile" << endl;
	cout << "All files must be one of the following formats: .wav .aiff .snd" << endl;   
}

bool checkExtensions(char* filename) 
{
	string extension = SoundFile::parseExtension(filename);

	if (extension.compare(".wav") == 0 ||
		extension.compare(".aiff") == 0 ||
		extension.compare(".snd") == 0)
		return true;

	return false;
}

// Returns true if arguments are valid. False otherwise.
bool checkArgs(int argc, char* argv[])
{
	/* Check number of arguments and extensions */
	if (argc != 4 || !checkExtensions(argv[1]) || !checkExtensions(argv[2]) || !checkExtensions(argv[3])) {
		usage();
		return false;
	}

	return true;
}

int main(int argc, char* argv[])
{
#ifdef TESTS
	RegressionTest::runAllTests();
	TestConvolver::runAllTests();
#endif
		
	/* Ensure dry, IR, and output file names are provided */
	if (! checkArgs(argc, argv))
		return 1;

	/* Read wave files */
	SoundFile* dryRecording = SoundFile::create(argv[1]);
	SoundFile* impulseResponse = SoundFile::create(argv[2]);

	/* End program if any input file is corrupted or nonexistant */
	if (dryRecording == nullptr || impulseResponse == nullptr)
		return 1;

	cout << "DR Size: " << dryRecording->getDataSize() << " IR Size: " << impulseResponse->getDataSize() << endl;

	/* Store result of convolution */
	short* result;
	int numSamples;

#ifndef FFT
	/* Perform Time-Domain Convolution */
	int resultSize;
	if (impulseResponse->getNumChannels() == 1) {
		resultSize = dryRecording->getDataSize() + impulseResponse->getDataSize() - 1;
		numSamples = resultSize;
	}
	else {
		resultSize = (dryRecording->getDataSize() + impulseResponse->getDataSize()/2 - 1)*2;
		numSamples = resultSize/2;
	}

	result = new short[resultSize];

	Convolver::convolve(dryRecording, impulseResponse, result, resultSize);

	/* Save resulting .wav file */
	cout << "Impulse Response num channels: " << impulseResponse->getNumChannels() << endl;
	cout << "Number of samples: " << resultSize << endl;
	Wave::save(argv[3], impulseResponse->getNumChannels(), numSamples, dryRecording->getBitsPerSample(), 
					dryRecording->getSampleRate(), result, resultSize);
#else
	/* Perform FFT Convolution */
	int structuredSize = 1;
	double *R;

	/* Normalize the input signal to -1 and +1 */
	double* dryNormalized = new double[dryRecording->getDataSize()];
	double* irNormalized = new double[impulseResponse->getDataSize()];
	for (int i = 0; i < dryRecording->getDataSize(); i++) 
		dryNormalized[i] = (double) dryRecording->getData()[i]/32768;
	for (int i = 0; i < impulseResponse->getDataSize(); i++)
		irNormalized[i] = (double) impulseResponse->getData()[i]/32768;

	if (impulseResponse->getNumChannels() == 1) {
		while ( structuredSize < dryRecording->getDataSize() || structuredSize < impulseResponse->getDataSize() )
			structuredSize *= 2;	// TODO: Replace with bit shift

		numSamples = structuredSize;

		/* Zero-pad normalized signals and convert to Frequency Domain */		
		double *X = new double[structuredSize * 2];
		double *H = new double[structuredSize * 2];
		R = new double[structuredSize * 2];

		Convolver::zeroPadAndTimeToFreqDomain(dryNormalized, dryRecording->getDataSize(), X, structuredSize);
		Convolver::zeroPadAndTimeToFreqDomain(irNormalized, impulseResponse->getDataSize(), H, structuredSize);

		/* Perform Frequency-Domain Convolution */
		Convolver::fftConvolve(X, H, R, structuredSize);

		/* Convert Frequency-Domain back to Time-Domain */
		Convolver::four1(R-1, structuredSize, -1);

		delete[] X;
		delete[] H;
	}
	else {
		/* Split stereo impulse response into left and right channels */
		short* irLeft = new short[impulseResponse->getDataSize()/2];
		short* irRight = new short[impulseResponse->getDataSize()/2];
		impulseResponse->splitChannels(irLeft, irRight, impulseResponse->getDataSize()/2);

		/* Normalize both left and right channels of IR */
		double* irLeftNormalized = new double[impulseResponse->getDataSize()/2];
		double* irRightNormalized = new double[impulseResponse->getDataSize()/2];
		for (int i = 0; i < impulseResponse->getDataSize()/2; i++) 
			irLeftNormalized[i] = (double) irLeft[i]/32768;
		for (int i = 0; i < impulseResponse->getDataSize()/2; i++)
			irRightNormalized[i] = (double) irRight[i]/32768;

		while ( structuredSize < dryRecording->getDataSize() || structuredSize < impulseResponse->getDataSize()/2 )
			structuredSize *= 2;	// TODO: Replace with bit shift

		double* X = new double[structuredSize * 2];
		double* H = new double[structuredSize * 2];
		double* RLeft = new double[structuredSize * 2];
		double* RRight = new double[structuredSize * 2];

		/* Transform Time-Domain to Frequency-Domain and FFT Convolve Left Channel */
		Convolver::zeroPadAndTimeToFreqDomain(dryNormalized, dryRecording->getDataSize(), X, structuredSize);
		Convolver::zeroPadAndTimeToFreqDomain(irLeftNormalized, impulseResponse->getDataSize()/2, H, structuredSize);
		Convolver::fftConvolve(X, H, RLeft, structuredSize);

		/* Transform Time-Domain to Frequency-Domain and FFT Convolve Right Channel */
		Convolver::zeroPadAndTimeToFreqDomain(irRightNormalized, impulseResponse->getDataSize()/2, H, structuredSize);
		Convolver::fftConvolve(X, H, RRight, structuredSize);

		delete[] irLeft;
		delete[] irRight;
		delete[] X;
		delete[] H;

		/* Convert Frequency-Domain back to Time-Domain */
		Convolver::four1(RLeft-1, structuredSize, -1);
		Convolver::four1(RRight-1, structuredSize, -1);

		/* Interleave left and right channel data */
		R = new double[structuredSize*4];
		impulseResponse->interleaveComplex(RLeft, RRight, structuredSize*2, R);

		/* Twice as many data values since there are two channels, to times two to be consistent
		   with the case where there is only one channel. */
		numSamples = structuredSize;
		structuredSize *= 2;

		delete[] RLeft;
		delete[] RRight;
	}

	/* Divide everything by N and find min/max */
	double min = R[0]/(double)(structuredSize*2), 
		   max = R[0]/(double)(structuredSize*2);

	for (int i = 0; i < structuredSize*2; i++) {
		R[i] /= (structuredSize*2);

		if (R[i] < min)
			min = R[i];
		if (R[i] > max)
			max = R[i];
	}

	for (int i = 0; i < structuredSize*2; i++) {
		R[i] = Convolver::normalize(R[i], min, max, -1.0, 1.0);
	}

	/* Scale result back up to short */
	result = new short[structuredSize];
	for (int i = 0; i < structuredSize*2; i+=2)
		result[i/2] = Convolver::symmetricalRound(R[i]*32767.0);//Convolver::normalize(R[i], min, max, inputSignalMin, inputSignalMax);

	cout << structuredSize << " " << numSamples << " " << dryRecording->getDataSize() << endl;
	cout << "P: ask Manzara " << dryRecording->getDataSize() + impulseResponse->getDataSize() - 1 << endl;
	cout << R[0] << R[0]*32767<< endl;
	system("pause");

	/* Save resulting .wav file */
	Wave::save(argv[3], impulseResponse->getNumChannels(), numSamples, dryRecording->getBitsPerSample(), 
					dryRecording->getSampleRate(), result, structuredSize);

	delete[] R;
#endif
	


	/* Free memory */
	delete dryRecording;
	delete impulseResponse;
	delete[] result;

	return 0;
}