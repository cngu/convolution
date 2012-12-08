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
	//RegressionTest::runAllTests(); 
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

	cout << "DR Size: " << dry->getDataSize() << " IR Size: " << ir->getDataSize() << endl;

	short* result;	// Resulting data of convolution to write to file
	int P;			// Length of result

	/* Normalize the input data to -1 and +1 */
	double* dryNormalized = new double[dry->getDataSize()];
	double* irNormalized = new double[ir->getDataSize()];
	Convolver::dataToSignal(dry->getData(), dry->getDataSize(), dry->getAbsMinValue(), dryNormalized);
	Convolver::dataToSignal(ir->getData(), ir->getDataSize(), ir->getAbsMinValue(), irNormalized);
		
	if (ir->getNumChannels() == 1) {
		P = dry->getDataSize() + ir->getDataSize() - 1;

		/* Perform time domain convolution */
		result = new short[P];

#ifndef FFT
		Convolver::convolve(dryNormalized, dry->getDataSize(), irNormalized, ir->getDataSize(), result, P);
#else
		Convolver::fftConvolve(dryNormalized, dry->getDataSize(), irNormalized, ir->getDataSize(), result, P);
#endif

		/* Save resulting .wav file */
		SoundFile::save(argv[3], ir->getNumChannels(), dry->getBitsPerSample(), dry->getSampleRate(), result, P);
	}
	else if (ir->getNumChannels() == 2) {
		/* P = N+(M/2)-1, where P is the length of each half of this two-channel convolution */
		P = dry->getDataSize() + ir->getDataSize()/2 - 1;

		/* Split normalized stereo impulse response into left and right channels */
		double* irLeftNormalized = new double[ir->getDataSize()/2];
		double* irRightNormalized = new double[ir->getDataSize()/2];
		for (int i = 0; i < ir->getDataSize()/2; i++) {
			irLeftNormalized[i] = irNormalized[i*2];
			irRightNormalized[i] = irNormalized[i*2 + 1];
		}

		/* Perform time domain convolution */
		short* resultLeft = new short[P];
		short* resultRight = new short[P];

#ifndef FFT
		Convolver::convolve(dryNormalized, dry->getDataSize(), irLeftNormalized, ir->getDataSize()/2, resultLeft, P);
		Convolver::convolve(dryNormalized, dry->getDataSize(), irRightNormalized, ir->getDataSize()/2, resultRight, P);
#else
		Convolver::fftConvolve(dryNormalized, dry->getDataSize(), irLeftNormalized, ir->getDataSize()/2, resultLeft, P);
		Convolver::fftConvolve(dryNormalized, dry->getDataSize(), irRightNormalized, ir->getDataSize()/2, resultRight, P);
#endif

		/* Interleave left and right channel data */
		result = new short[P*2];
		for (int i = 0; i < P*2; i+=2) {
			result[i] = resultLeft[i/2];
			result[i+1] = resultRight[i/2];
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