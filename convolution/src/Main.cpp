#include <algorithm>
#include <iostream>
#include <string>

#include "Convolver.h"
#include "SoundFile.h"
#include "Wave.h"
#include "Snd.h"
#include "Aiff.h"
#include "gtest\gtest.h"

// Control directives
//#define FFT		// Uncomment to use Frequency-Domain Convolution rather than Time Domain
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
	/* Run tests */
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	return 0;
#endif

	/* Ensure dry, IR, and output file names are provided */
	if (! checkArgs(argc, argv))
		return 1;

	/* Read wave files */
	SoundFile* dryRecording = SoundFile::create(argv[1]);
	SoundFile* impulseResponse = SoundFile::create(argv[2]);

	/* End program if any input file is corrupted or nonexistant */
	// TODO: do this check in between createWave's above. If first 
	// is corrupted, no point in creating Wave objects for the rest
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

	if (impulseResponse->getNumChannels() == 1) {
		while ( structuredSize < dryRecording->getDataSize() || structuredSize < impulseResponse->getDataSize() )
			structuredSize *= 2;	// TODO: Replace with bit shift

		numSamples = structuredSize;

		double *F = new double[structuredSize * 2];
		double *G = new double[structuredSize * 2];
		R = new double[structuredSize * 2];

		/* Transform Time-Domain to Frequency-Domain */
		Convolver::timeDomainToFreqDomain(dryRecording->getData(), dryRecording->getDataSize(), F, structuredSize);
		Convolver::timeDomainToFreqDomain(impulseResponse->getData(), impulseResponse->getDataSize(), G, structuredSize);

		/* Perform Frequency-Domain Convolution */
		Convolver::fftConvolve(F, G, R, structuredSize);

		/* Convert Frequency-Domain back to Time-Domain */
		Convolver::four1(R-1, structuredSize, -1);

		delete[] F;
		delete[] G;
	}
	else {
		/* Split stereo impulse response into left and right channels */
		short* irLeft = new short[impulseResponse->getDataSize()/2];
		short* irRight = new short[impulseResponse->getDataSize()/2];
		impulseResponse->splitChannels(irLeft, irRight, impulseResponse->getDataSize()/2);

		while ( structuredSize < dryRecording->getDataSize() || structuredSize < impulseResponse->getDataSize()/2 )
			structuredSize *= 2;	// TODO: Replace with bit shift

		double *F = new double[structuredSize * 2];
		double *G = new double[structuredSize * 2];
		double *RLeft = new double[structuredSize * 2];
		double *RRight = new double[structuredSize * 2];

		/* Transform Time-Domain to Frequency-Domain and FFT Convolve Left Channel */
		Convolver::timeDomainToFreqDomain(dryRecording->getData(), dryRecording->getDataSize(), F, structuredSize);
		Convolver::timeDomainToFreqDomain(irLeft, impulseResponse->getDataSize()/2, G, structuredSize);
		Convolver::fftConvolve(F, G, RLeft, structuredSize);

		/* Transform Time-Domain to Frequency-Domain and FFT Convolve Right Channel */
		Convolver::timeDomainToFreqDomain(irRight, impulseResponse->getDataSize()/2, G, structuredSize);
		Convolver::fftConvolve(F, G, RRight, structuredSize);

		delete[] irLeft;
		delete[] irRight;
		delete[] F;
		delete[] G;

		/* Convert Frequency-Domain back to Time-Domain */
		Convolver::four1(RLeft-1, structuredSize, -1);
		Convolver::four1(RRight-1, structuredSize, -1);

		/* Interleave left and right channel data */
		R = new double[structuredSize*4];
		impulseResponse->interleaveComplex(RLeft, RRight, structuredSize*2, R);

		/* Shuffle all real data to the left half */
		/*for (int i = 0; i < structuredSize; i++) {
			RLeft[i] = RLeft[i*2];
			RRight[i] = RRight[i*2];
		}
		*/

		/* Twice as many data values since there are two channels, to times two to be consistent
		   with the case where there is only one channel. */
		numSamples = structuredSize;
		structuredSize *= 2;

		delete[] RLeft;
		delete[] RRight;
	}

	/* Divide everything by N and find min/max */
	double min = R[0]/structuredSize, max = R[0]/structuredSize;
	for (int i = 0; i < structuredSize*2; i+=2) {
		R[i] /= structuredSize;
		if (R[i] < min)
			min = R[i];
		if (R[i] > max)
			max = R[i];
	}

	/* Normalize result to be between -32768 and 32767 */
	result = new short[structuredSize];
	for (int i = 0; i < structuredSize*2; i+=2)
		result[i/2] = Convolver::normalize(R[i], min, max, -32768, 32767);

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