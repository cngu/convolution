#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "Wave.h"

using namespace std;

Wave::Wave(char* wavePath)
{
	loadWave(wavePath);
}

Wave::~Wave()
{
	delete[] data;
}

void Wave::readShortChunk(ifstream& ifs, int& offset, short& value)
{
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &value, sizeof(value) );
	offset += sizeof(value);
}

void Wave::readIntChunk(ifstream& ifs, int& offset, int& value)
{
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &value, sizeof(value) );
	offset += sizeof(value);
}

void Wave::loadWave(char* loadPath)
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
		min = data[0];
		max = data[0];
		for (int i = 0; i < subchunk2Size; i+=2) {
			// TODO: Shift left 8 instead
			sample = (short) ( (unsigned char) rawData[i] );
			sample += (short) ( (unsigned char) rawData[i+1]) * 256;
			// TODO: Shift RIGHT 1 instead
			data[i/2] = sample;

			if (data[i/2] < min)
				min = data[i/2];
			if (data[i/2] > max)
				max = data[i/2];
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
