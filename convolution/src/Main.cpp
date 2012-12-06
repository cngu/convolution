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
//#define FFT			// Uncomment to use Frequency-Domain Convolution rather than Time Domain
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
		
	/* Ensure dry, IR, and output file names are provided */
	if (! checkArgs(argc, argv))
		return 1;

	/* Read wave files */
	SoundFile* dry = SoundFile::create(argv[1]);
	SoundFile* ir = SoundFile::create(argv[2]);

	/* End program if any input file is corrupted or nonexistant */
	if (dry == nullptr || ir == nullptr)
		return 1;

	cout << "DR Size: " << dry->getDataSize() << " IR Size: " << ir->getDataSize() << endl;

	/* Lower bound values of dry and impulse. Used to normalize their respective signals. */
	short* result;
	int P = dry->getDataSize() + ir->getDataSize() - 1;

#ifndef FFT
	/* Perform Time-Domain Convolution */
	if (ir->getNumChannels() == 1) {
		result = new short[P];
		Convolver::convolve(dry, ir, result, P);

		/* Save resulting .wav file */
		Wave::save(argv[3], ir->getNumChannels(), P, dry->getBitsPerSample(), dry->getSampleRate(), result, P);
	}
	else { //if (ir->getNumChannels() == 2) {
		P = dry->getDataSize() + ir->getDataSize()/2 - 1;
		result = new short[P*2];
		Convolver::convolve(dry, ir, result, P*2);

		/* Save resulting .wav file */
		Wave::save(argv[3], ir->getNumChannels(), P, dry->getBitsPerSample(), dry->getSampleRate(), result, P*2);
	}
#else
	/* Perform FFT Convolution */
	int structuredSize = 1;
	double* X;
	double* H;
	double* R;

	/* Normalize the input signal to -1 and +1 */
	double* dryNormalized = new double[dry->getDataSize()];
	double* irNormalized = new double[ir->getDataSize()];
	Convolver::dataToSignal(dry->getData(), dry->getDataSize(), dry->getAbsMinValue(), dryNormalized);
	Convolver::dataToSignal(ir->getData(), ir->getDataSize(), ir->getAbsMinValue(), irNormalized);

	if (ir->getNumChannels() == 1) {
		while (structuredSize < P)
			structuredSize *= 2;	// TODO: Replace with bit shift

		X = new double[structuredSize * 2];
		H = new double[structuredSize * 2];
		R = new double[structuredSize * 2];

		/* Zero-pad normalized signals and convert to Frequency Domain */	
		Convolver::zeroPadAndTimeToFreqDomain(dryNormalized, dry->getDataSize(), X, structuredSize);
		Convolver::zeroPadAndTimeToFreqDomain(irNormalized, ir->getDataSize(), H, structuredSize);

		/* Perform Frequency-Domain Convolution */
		Convolver::fftConvolve(X, H, R, structuredSize);

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
		result = new short[P];
		Convolver::complexSignalToData(R, P*2, dry->getMaxValue(), result);

		string extension = SoundFile::parseExtension(argv[3]);
		if (extension.compare(".wav") == 0)
			Wave::save(argv[3], ir->getNumChannels(), P, dry->getBitsPerSample(), dry->getSampleRate(), result, P);
		else if (extension.compare(".snd") == 0)
			Snd::save(argv[3], ((Snd*)dry)->dataOffset, dry->getSampleRate(), ir->getNumChannels(), result, P);
		else if (extension.compare(".aiff") == 0)
			Aiff::save(argv[3], ir->getNumChannels(), dry->getSampleRate(), result, P);

		delete[] R;
	}
	else {
		/* Split normalized stereo impulse response into left and right channels */
		double* irLeftNormalized = new double[ir->getDataSize()/2];
		double* irRightNormalized = new double[ir->getDataSize()/2];
		
		for (int i = 0; i < ir->getDataSize()/2; i++) {
			irLeftNormalized[i] = irNormalized[i*2];
			irRightNormalized[i] = irNormalized[i*2 + 1];
		}

		P = dry->getDataSize() + ir->getDataSize()/2 - 1;
		while (structuredSize < P)
			structuredSize *= 2;	// TODO: Replace with bit shift

		X = new double[structuredSize * 2];
		H = new double[structuredSize * 2];
		double* RLeft = new double[structuredSize * 2];
		double* RRight = new double[structuredSize * 2];

		/* Transform Time-Domain to Frequency-Domain and FFT Convolve Left Channel */
		Convolver::zeroPadAndTimeToFreqDomain(dryNormalized, dry->getDataSize(), X, structuredSize);
		Convolver::zeroPadAndTimeToFreqDomain(irLeftNormalized, ir->getDataSize()/2, H, structuredSize);
		Convolver::fftConvolve(X, H, RLeft, structuredSize);

		/* Transform Time-Domain to Frequency-Domain and FFT Convolve Right Channel */
		Convolver::zeroPadAndTimeToFreqDomain(irRightNormalized, ir->getDataSize()/2, H, structuredSize);
		Convolver::fftConvolve(X, H, RRight, structuredSize);

		/* Convert Frequency-Domain back to Time-Domain */
		Convolver::four1(RLeft-1, structuredSize, -1);
		Convolver::four1(RRight-1, structuredSize, -1);

		/* Divide everything by N and find min/max */
		double lMin = RLeft[0]/(double)structuredSize, 
			   lMax = RLeft[0]/(double)structuredSize,
			   rMin = RRight[0]/(double)structuredSize,
			   rMax = RRight[0]/(double)structuredSize;

		for (int i = 0; i < structuredSize*2; i+=2) {
			RLeft[i] /= (double)structuredSize;
			RRight[i] /= (double)structuredSize;

			if (RLeft[i] < lMin)
				lMin = RLeft[i];
			if (RLeft[i] > lMax)
				lMax = RLeft[i];
			if (RRight[i] < rMin)
				rMin = RRight[i];
			if (RRight[i] > rMax)
				rMax = RRight[i];
		}
	
		// Do we need to loop up to structuredSize*2? Same with loop immediately above
		for (int i = 0; i < structuredSize*2; i+=2) {
			RLeft[i] = Convolver::normalize(RLeft[i], lMin, lMax, -1.0, 1.0);
			RRight[i] = Convolver::normalize(RRight[i], rMin, rMax, -1.0, 1.0);
		}

		short* resultLeft = new short[P];
		short* resultRight = new short[P];
		Convolver::complexSignalToData(RLeft, P*2, dry->getMaxValue(), resultLeft);
		Convolver::complexSignalToData(RRight, P*2, dry->getMaxValue(), resultRight);

		/* Interleave left and right channel data */
		result = new short[P*2];
		for (int i = 0; i < P*2; i+=2) {
			result[i] = resultLeft[i/2];
			result[i+1] = resultRight[i/2];
		}

		/* Twice as many data values since there are two channels, to times two to be consistent
		   with the case where there is only one channel. */
		//P *= 2;
		//structuredSize *= 2;

		string extension = SoundFile::parseExtension(argv[3]);
		if (extension.compare(".wav") == 0)
			Wave::save(argv[3], ir->getNumChannels(), P, dry->getBitsPerSample(), dry->getSampleRate(), result, P*2);
		else if (extension.compare(".snd") == 0)
			Snd::save(argv[3], ((Snd*)dry)->dataOffset, dry->getSampleRate(), ir->getNumChannels(), result, P*2);
		else if (extension.compare(".aiff") == 0)
			Aiff::save(argv[3], ir->getNumChannels(), dry->getSampleRate(), result, P*2);

		delete[] RLeft;
		delete[] RRight;
		delete[] resultLeft;
		delete[] resultRight;
	}

	/* Free memory */
	delete dry;
	delete ir;
	delete[] X;
	delete[] H;
	delete[] result;

#endif
	
	

	return 0;
}