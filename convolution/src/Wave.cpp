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

void Wave::loadWave(char* loadPath)
{
	ifstream ifs(loadPath, ios::in | ios::binary);
	if (ifs.fail()) {
		throw invalid_argument("Wave::readWave: " + string(loadPath) + " could not be opened.");
	}

	int offset = 4;		// Skip ChunkID, start at ChunkSize
	
	cout << "Reading chunkSize " << sizeof(chunkSize) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &chunkSize, sizeof(chunkSize) );
	offset += sizeof(chunkSize);
	cout << chunkSize << endl;

	offset += sizeof(format);
	offset += sizeof(subchunk1ID);

	cout << "Reading subchunk1Size" << sizeof(subchunk1Size) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &subchunk1Size, sizeof(subchunk1Size) );
	offset += sizeof(subchunk1Size);
	cout << subchunk1Size << endl;

	cout << "Reading audioFormat" << sizeof(audioFormat) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &audioFormat, sizeof(audioFormat) );
	offset += sizeof(audioFormat);
	cout << audioFormat << endl;

	cout << "Reading numChannels" << sizeof(numChannels) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &numChannels, sizeof(numChannels) );
	offset += sizeof(numChannels);
	cout << numChannels << endl;

	cout << "Reading sampleRate" << sizeof(sampleRate) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &sampleRate, sizeof(sampleRate) );
	offset += sizeof(sampleRate);
	cout << sampleRate << endl;

	cout << "Reading byteRate" << sizeof(byteRate) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &byteRate, sizeof(byteRate) );
	offset += sizeof(byteRate);
	cout << byteRate << endl;

	cout << "Reading blockAlign" << sizeof(blockAlign) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &blockAlign, sizeof(blockAlign) );
	offset += sizeof(blockAlign);
	cout << blockAlign << endl;

	cout << "Reading bitsPerSample" << sizeof(bitsPerSample) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &bitsPerSample, sizeof(bitsPerSample) );
	offset += sizeof(bitsPerSample);
	cout << bitsPerSample << endl;

	offset += sizeof(subchunk2ID);

	cout << "Reading subchunk2Size" << sizeof(subchunk2Size) << " bytes into address: " << offset << endl;
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &subchunk2Size, sizeof(subchunk2Size) );
	offset += sizeof(subchunk2Size);
	cout << subchunk2Size << endl;

	char* rawData = new char[subchunk2Size];
	ifs.seekg(offset, ios::beg);
	ifs.read(rawData, subchunk2Size);

	// TODO: If desperate, put this if inside the loop, and then optimize it back out like it is now
	if (bitsPerSample == 16) {
		// TODO: Shift RIGHT 1 instead
		dataSize = subchunk2Size/2;
		data = new short[dataSize];

		short sample;
		min = 32767;
		max = -32768;
		for (int i = 0; i < subchunk2Size; i+=2) {
			// TODO: Shift left 8 instead
			sample = (short) ( (unsigned char) rawData[i] );
			sample += (short) ( (unsigned char) rawData[i+1]) * 256;
			// TODO: Shift RIGHT 1 instead
			data[i/2] = sample;

			if (data[i/2] < min)
				min = data[i/2];
			else if (data[i/2] > max)
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
