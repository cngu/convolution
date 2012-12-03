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
