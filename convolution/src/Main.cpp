#include <iostream>
#include <string>
#include <climits>

#include "RegressionTest.h"
#include "TestConvolver.h"

#include "Convolver.h"
#include "SoundFile.h"
#include "Wave.h"
#include "Snd.h"
#include "Aiff.h"

// Control directives
#define FFT			// Uncomment to use Frequency-Domain Convolution rather than Time Domain
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
	cout << "===== RUNNING TESTS =====" << endl;
	RegressionTest::runAllTests(); 
	TestConvolver::runAllTests();
	cout << "==== TESTS COMPLETED ====" << endl;
	system("pause");
#endif
	
	/* Ensure that valid dry, IR, and output file names are provided */
	if (! checkArgs(argc, argv))
		return 1;

	/* Read wave files */
	SoundFile* dry = SoundFile::create(argv[1]);
	SoundFile* ir = SoundFile::create(argv[2]);

	/* End program if any input file is corrupted or nonexistant */
	if (dry == nullptr || ir == nullptr)
		return 1;

	int dryDataSize = dry->getDataSize();
	int irDataSize = ir->getDataSize();

	cout << "DR Size: " << dryDataSize << " IR Size: " << irDataSize << endl;

	int P;			// Length of result
	short* result;	// Resulting data of convolution to write to file
	
	/* Normalize the input data to -1 and +1 */
	double* dryNormalized = new double[dryDataSize];
	double* irNormalized = new double[irDataSize];
	Convolver::dataToSignal(dry->getData(), dryDataSize, dry->getAbsMinValue(), dryNormalized);
	Convolver::dataToSignal(ir->getData(), irDataSize, ir->getAbsMinValue(), irNormalized);
	
	if (ir->getNumChannels() == 1) {
		P = dryDataSize + irDataSize - 1;

		/* Perform time domain convolution */
		result = new short[P];

#ifndef FFT
		Convolver::convolve(dryNormalized, dryDataSize, irNormalized, irDataSize, result, P);
#else
		Convolver::fftConvolve(dryNormalized, dryDataSize, irNormalized, irDataSize, result, P);
#endif

		/* Save resulting .wav file */
		SoundFile::save(argv[3], ir->getNumChannels(), dry->getBitsPerSample(), dry->getSampleRate(), result, P);
	}
	else if (ir->getNumChannels() == 2) {
		/* Calculate half size of impulse response ONCE and store in halfM */
		int halfM = irDataSize >> 1;

		/* P = N+(M/2)-1, where P is the length of each half of this two-channel convolution */
		P = dryDataSize + halfM - 1;

		/* Split normalized stereo impulse response into left and right channels */
		double* irLeftNormalized = new double[halfM];
		double* irRightNormalized = new double[halfM];

		for (int i = 0, i2 = 0; i < halfM; i++) {
			i2 = i << 1;
			irLeftNormalized[i] = irNormalized[i2];
			irRightNormalized[i] = irNormalized[i2 + 1];
		}

		/* Perform time domain convolution */
		short* resultLeft = new short[P];
		short* resultRight = new short[P];

#ifndef FFT
		Convolver::convolve(dryNormalized, dryDataSize, irLeftNormalized, halfM, resultLeft, P);
		Convolver::convolve(dryNormalized, dryDataSize, irRightNormalized, halfM, resultRight, P);
#else
		Convolver::fftConvolve(dryNormalized, dryDataSize, irLeftNormalized, halfM, resultLeft, P);
		Convolver::fftConvolve(dryNormalized, dryDataSize, irRightNormalized, halfM, resultRight, P);
#endif

		/* Interleave left and right channel data */
		result = new short[P*2];
		for (int i = 0, halfI = 0; i < P*2; i+=2) {
			halfI = i >> 1;
			result[i] = resultLeft[halfI];
			result[i+1] = resultRight[halfI];
		}

		/* Save resulting .wav file */
		SoundFile::save(argv[3], ir->getNumChannels(), dry->getBitsPerSample(), dry->getSampleRate(), result, P*2);

		delete[] irLeftNormalized;
		delete[] irRightNormalized;
		delete[] resultLeft;
		delete[] resultRight;
	}
	else {
		cout << ir->getNumChannels() << " channels are not supported.\nAborting program." << endl;
	}

	delete[] dryNormalized;
	delete[] irNormalized;
	delete[] result;

	return 0;
}