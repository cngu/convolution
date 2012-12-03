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
#define FFT		// Uncomment to use Frequency-Domain Convolution rather than Time Domain
//#define TESTS		// Uncomment to run tests

#define BITS_PER_SAMPLE 16
#define SAMPLE_RATE 44100

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

/* Written By Leonard Manzara */
size_t fwriteIntLSB(int data, FILE *stream)
{
	unsigned char array[4];

    array[3] = (unsigned char)((data >> 24) & 0xFF);
    array[2] = (unsigned char)((data >> 16) & 0xFF);
    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    return fwrite(array, sizeof(unsigned char), 4, stream);
}

/* Written By Leonard Manzara */
size_t fwriteShortLSB(short int data, FILE *stream)
{
	unsigned char array[2];

    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    return fwrite(array, sizeof(unsigned char), 2, stream);
}

/* Written By Leonard Manzara. Modified by Abbas Sarraf. */
void writeWaveFileHeader(int channels, int numberSamples, int bitsPerSample, 
					     double sampleRate, FILE *outputFile)
{
	/*  Calculate the total number of bytes for the data chunk  */
	int dataChunkSize = channels * numberSamples * (bitsPerSample / 8);
	
	/*  Calculate the total number of bytes for the form size  */
	int formSize = 36 + dataChunkSize;
	
	/*  Calculate the total number of bytes per frame  */
	short int frameSize = channels * (bitsPerSample / 8);
	
	/*  Calculate the byte rate  */
	int bytesPerSecond = (int)ceil(sampleRate * frameSize);
	
	/*  Write header to file  */
	/*  Form container identifier  */
	fputs("RIFF", outputFile);
	
	/*  Form size  */
	fwriteIntLSB(formSize, outputFile);
	
	/*  Form container type  */
	fputs("WAVE", outputFile);
	
	/*  Format chunk identifier (Note: space after 't' needed)  */
	fputs("fmt ", outputFile);
	
	/*  Format chunk size (fixed at 16 bytes)  */
	fwriteIntLSB(16, outputFile);
	
	/*  Compression code:  1 = PCM  */
	fwriteShortLSB(1, outputFile);
	
	/*  Number of channels  */
	fwriteShortLSB((short)channels, outputFile);
	
	/*  Output Sample Rate  */
	fwriteIntLSB((int)sampleRate, outputFile);
	
	/*  Bytes per second  */
	fwriteIntLSB(bytesPerSecond, outputFile);
	
	/*  Block alignment (frame size)  */
	fwriteShortLSB(frameSize, outputFile);
	
	/*  Bits per sample  */
	fwriteShortLSB(bitsPerSample, outputFile);
	
	/*  Sound Data chunk identifier  */
	fputs("data", outputFile);
	
	/*  Chunk size  */
	fwriteIntLSB(dataChunkSize, outputFile);
}

void writeWaveFileData(short data[], int len, FILE* outputFile)
{
	for (int i = 0; i < len; i++) {
		fwriteShortLSB(data[i], outputFile);
	}
}

void saveWaveFile(char* outputFile, int channels, int numberSamples, int bitsPerSample, 
				  double sampleRate, short data[], int dataLen) 
{
	FILE *outputWaveFile = fopen(outputFile, "wb");
	writeWaveFileHeader(channels, numberSamples, bitsPerSample, sampleRate, outputWaveFile);
	writeWaveFileData(data, dataLen, outputWaveFile);
	fclose(outputWaveFile);
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

#ifndef FFT
	/* Perform Time-Domain Convolution */
	int resultSize = dryRecording->getDataSize() + impulseResponse->getDataSize() - 1;
	result = new short[resultSize];
	Convolver::convolve(dryRecording, impulseResponse, result, resultSize);

	/* Save resulting .wav file */
	saveWaveFile(argv[3], impulseResponse->getNumChannels(), resultSize, BITS_PER_SAMPLE, 
					SAMPLE_RATE, result, resultSize);
#else
	/* Perform FFT Convolution */
	int structuredSize = 1;
	while ( structuredSize < dryRecording->getDataSize() || structuredSize < impulseResponse->getDataSize() )
		structuredSize *= 2;	// TODO: Replace with bit shift

	double *F = new double[structuredSize * 2];
	double *G = new double[structuredSize * 2];
	double *R = new double[structuredSize * 2];

	/* Transform Time-Domain to Frequency-Domain */
	Convolver::timeDomainToFreqDomain(dryRecording->getData(), dryRecording->getDataSize(), F, structuredSize);
	Convolver::timeDomainToFreqDomain(impulseResponse->getData(), impulseResponse->getDataSize(), G, structuredSize);

	/* Perform Frequency-Domain Convolution */
	Convolver::fftConvolve(F, G, R, structuredSize);

	/* Convert Frequency-Domain back to Time-Domain */
	Convolver::four1(R-1, structuredSize, -1);

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
	saveWaveFile(argv[3], impulseResponse->getNumChannels(), structuredSize, BITS_PER_SAMPLE, 
					SAMPLE_RATE, result, structuredSize);

	delete[] F;
	delete[] G;
	delete[] R;
#endif
	


	/* Free memory */
	delete dryRecording;
	delete impulseResponse;
	delete[] result;

	return 0;
}