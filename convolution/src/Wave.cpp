#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "Wave.h"

using namespace std;

Wave::Wave(char* wavePath)
{
	load(wavePath);
}

Wave::~Wave()
{
	delete[] data;
}

int Wave::getNumChannels()
{
	return numChannels;
}

int Wave::getSampleRate()
{
	return sampleRate;
}

int Wave::getDataSize()
{
	return dataSize;
}

short Wave::getBitsPerSample()
{
	return bitsPerSample;
}

short* Wave::getData()
{
	return data;
}

void Wave::load(char* loadPath)
{
	ifstream ifs(loadPath, ios::in | ios::binary);
	if (ifs.fail()) {
		throw invalid_argument("Wave::readWave: " + string(loadPath) + " could not be opened.");
	}

	/* Read Header */
	int offset = 4;		// Skip ChunkID, start at ChunkSize
	readIntChunk(ifs, offset, chunkSize);
	offset += sizeof(format);
	offset += sizeof(subchunk1ID);
	readIntChunk(ifs, offset, subchunk1Size);
	readShortChunk(ifs, offset, audioFormat);
	readShortChunk(ifs, offset, numChannels);
	readIntChunk(ifs, offset, sampleRate);
	readIntChunk(ifs, offset, byteRate);
	readShortChunk(ifs, offset, blockAlign);
	readShortChunk(ifs, offset, bitsPerSample);
	offset += sizeof(subchunk2ID);
	readIntChunk(ifs, offset, subchunk2Size);

	/* Read Data */
	char* rawData = new char[subchunk2Size];
	ifs.seekg(offset, ios::beg);
	ifs.read(rawData, subchunk2Size);

	// TODO: If desperate, put this if inside the loop, and then optimize it back out like it is now
	if (bitsPerSample == 16) {
		// TODO: Shift RIGHT 1 instead
		dataSize = subchunk2Size/2;
		data = new short[dataSize];

		short sample;
		for (int i = 0; i < subchunk2Size; i+=2) {
			// TODO: Shift left 8 instead
			sample = (short) ( (unsigned char) rawData[i] );
			sample += (short) ( (unsigned char) rawData[i+1]) * 256;
			// TODO: Shift RIGHT 1 instead
			data[i/2] = sample;
		}
	}
	else {
		ifs.close();
		delete[] rawData;
		throw invalid_argument(bitsPerSample + " bits per sample is not supported.");
	}

	ifs.close();
	delete[] rawData;
}

/* Written By Leonard Manzara. Modified by Abbas Sarraf. */
void Wave::saveHeader(FILE *outputFile, int channels, int numberSamples, int bitsPerSample, double sampleRate)
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
	SoundFile::fwriteIntLSB(formSize, outputFile);
	
	/*  Form container type  */
	fputs("WAVE", outputFile);
	
	/*  Format chunk identifier (Note: space after 't' needed)  */
	fputs("fmt ", outputFile);
	
	/*  Format chunk size (fixed at 16 bytes)  */
	SoundFile::fwriteIntLSB(16, outputFile);
	
	/*  Compression code:  1 = PCM  */
	SoundFile::fwriteShortLSB(1, outputFile);
	
	/*  Number of channels  */
	SoundFile::fwriteShortLSB((short)channels, outputFile);
	
	/*  Output Sample Rate  */
	SoundFile::fwriteIntLSB((int)sampleRate, outputFile);
	
	/*  Bytes per second  */
	SoundFile::fwriteIntLSB(bytesPerSecond, outputFile);
	
	/*  Block alignment (frame size)  */
	SoundFile::fwriteShortLSB(frameSize, outputFile);
	
	/*  Bits per sample  */
	SoundFile::fwriteShortLSB(bitsPerSample, outputFile);
	
	/*  Sound Data chunk identifier  */
	fputs("data", outputFile);
	
	/*  Chunk size  */
	SoundFile::fwriteIntLSB(dataChunkSize, outputFile);
}

void Wave::saveData(FILE* outputFile, short data[], int len)
{
	for (int i = 0; i < len; i++) {
		SoundFile::fwriteShortLSB(data[i], outputFile);
	}
}

void Wave::save(char* outputFile, int channels, int numberSamples, int bitsPerSample, 
				     double sampleRate, short data[], int dataLen) 
{
	FILE *outputWaveFile = fopen(outputFile, "wb");
	saveHeader(outputWaveFile, channels, numberSamples, bitsPerSample, sampleRate);
	saveData(outputWaveFile, data, dataLen);
	fclose(outputWaveFile);
}