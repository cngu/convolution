#include <algorithm>
#include <iostream>
#include <memory>

#include "Convolver.h"
#include "Wave.h"
#include "gtest\gtest.h"

using namespace std;

void usage()
{
	cout << "Usage: convolve inputfile IRfile outputfile" << endl;
	cout << "All files must be one of the following formats: .wav .aiff .snd" << endl;   
}

// Returns true if arguments are valid. False otherwise.
bool checkArgs(int argc, char* argv[])
{
	// TODO: Check file extensions for .wav, .aiff, .snd
	// Initially use rfind for '.' and then substr. and then comparison (yet another loop)
	// Code tuning: unrolling
	if (argc != 4 && argc != 5) {
		usage();
		return false;
	}
	return true;
}

unique_ptr<Wave> createWave(char* filename)
{
	try {
		return unique_ptr<Wave>(new Wave(filename));
	}
	catch (invalid_argument& ia) {
		cout << ia.what() << endl;
		return nullptr;
	}
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



int main(int argc, char* argv[])
{
	cout << endl;

	/* Ensure dry, IR, and output file names are provided */
	if (! checkArgs(argc, argv))
		return 1;

	/* Read wave files */
	unique_ptr<Wave> dryRecording = createWave(argv[1]);
	unique_ptr<Wave> impulseResponse = createWave(argv[2]);

	/* End program if any input file is corrupted or nonexistant */
	// TODO: do this check in between createWave's above. If first 
	// is corrupted, no point in creating Wave objects for the rest
	if (dryRecording == nullptr || impulseResponse == nullptr)
		return 1;

	/* Perform Time-Domain Convolution */
	if (argv[4] == nullptr) {
		int resultSize = dryRecording->dataSize + impulseResponse->dataSize - 1;
		unique_ptr<short[]> result(new short[resultSize]);
		Convolver::convolve(dryRecording, impulseResponse, result.get(), resultSize);

		/* Create resulting .wav file */
		// Note: 
		//	- Always uses impulseResponses numChannels for Bonus#1
		//  - Always uses 16 bits per sample and 44.1 kHz sample rate as per assignment specifications
		FILE *outputWaveFile = fopen(argv[3], "wb");
		cout << "Writing to " << argv[3] << endl;
		writeWaveFileHeader(impulseResponse->numChannels, resultSize, 16, 44100, outputWaveFile);
		for (int i = 0; i < resultSize; i++) {
			fwriteShortLSB(result[i], outputWaveFile);
		}
		fclose(outputWaveFile);

		/* Free memory taken up by result */
		result = nullptr;
	}
	/* Perform FFT Convolution - Written by Abbas Sarraf. */
	else {
		int structuredSize = 1;
		while ( structuredSize < dryRecording->dataSize || structuredSize < impulseResponse->dataSize )
			structuredSize *= 2;	// TODO: Replace with bit shift

		double *F = new double[structuredSize * 2];
		double *G = new double[structuredSize * 2];
		double *R = new double[structuredSize * 2];

		/* Transform Time-Domain to Frequency-Domain */
		Convolver::timeDomainToFreqDomain(dryRecording->data, dryRecording->dataSize, F, structuredSize);
		Convolver::timeDomainToFreqDomain(impulseResponse->data, impulseResponse->dataSize, G, structuredSize);

		/* Perform Frequency-Domain Convolution */
		Convolver::fftConvolve(F, G, R, structuredSize);

		/* Convert Frequency-Domain back to Time-Domain */
		Convolver::four1(R-1, structuredSize, -1);

		/* Divide everything by N and find min/max */
		double min = R[0]/structuredSize, max = R[0]/structuredSize;
		for (int i = 0; i < structuredSize*2; i++) {
			R[i] /= structuredSize;
			if (R[i] < min)
				min = R[i];
			if (R[i] > max)
				max = R[i];
		}

		//min = *min_element(R, R+(structuredSize*2));
		//max = *max_element(R, R+(structuredSize*2));

		cout << "MIN: " << min << " MAX: " << max << endl;

		/* Normalize result in R to be between -32768 and 32767 */
		for (int i = 0; i < structuredSize*2; i++)
			R[i] = Convolver::normalize(R[i], min, max, -32768, 32767);

		FILE *outputWaveFile = fopen(argv[3], "wb");
		writeWaveFileHeader(impulseResponse->numChannels, structuredSize, 16, 44100, outputWaveFile);
		for (int i = 0; i < structuredSize*2; i++) {
			fwriteShortLSB(R[i], outputWaveFile);
		}
		fclose(outputWaveFile);

		delete[] F;
		delete[] G;
		delete[] R;
	}
	
	

	

	/* Run tests */
	//::testing::InitGoogleTest(&argc, argv);
	//RUN_ALL_TESTS();

	/* Destroy smart pointers which will release the managed object */
	dryRecording = nullptr;
	impulseResponse = nullptr;
	

	return 0;
}