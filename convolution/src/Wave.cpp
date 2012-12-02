#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <ios>

#include "Wave.h"

using namespace std;

Wave::Wave(char* wavePath)
{
	readWave(wavePath);
}

Wave::~Wave()
{

}

void Wave::readWave(char* wavePath)
{
	ifstream ifs(wavePath, ios_base::in | ios_base::binary);
	if (ifs.fail()) {
		throw invalid_argument("Wave::readWave: " + string(wavePath) + " could not be opened.");
	}

	int offset = sizeof(chunkSize);		// Skip ChunkID, start at ChunkSize
	
	cout << "Reading " << sizeof(chunkSize) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &chunkSize, sizeof(chunkSize) );
	offset += sizeof(chunkSize);

	offset += sizeof(format);
	offset += sizeof(subChunk1ID);

	cout << "Reading " << sizeof(subchunk1Size) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &subchunk1Size, sizeof(subchunk1Size) );
	offset += sizeof(subchunk1Size);

	cout << "Reading " << sizeof(audioFormat) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &audioFormat, sizeof(audioFormat) );
	offset += sizeof(audioFormat);

	cout << "Reading " << sizeof(numChannels) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &numChannels, sizeof(numChannels) );
	offset += sizeof(numChannels);

	cout << "Reading " << sizeof(sampleRate) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &sampleRate, sizeof(sampleRate) );
	offset += sizeof(sampleRate);

	cout << "Reading " << sizeof(byteRate) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &byteRate, sizeof(byteRate) );
	offset += sizeof(byteRate);

	cout << "Reading " << sizeof(blockAlign) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &blockAlign, sizeof(blockAlign) );
	offset += sizeof(blockAlign);

	cout << "Reading " << sizeof(bitsPerSample) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &bitsPerSample, sizeof(bitsPerSample) );
	offset += sizeof(bitsPerSample);

	offset += sizeof(subChunk2ID);

	cout << "Reading " << sizeof(subchunk2Size) << " bytes into address: " << offset << endl;
	ifs.seekg(offset);
	ifs.read( (char*) &subchunk2Size, sizeof(subchunk2Size) );
	offset += sizeof(subchunk2Size);

	char* rawData = new char[subchunk2Size];
	ifs.read( (char*) &rawData, subchunk2Size );

	// TODO: If desperate, put this if inside the loop, and then optimize it back out like it is now
	if (bitsPerSample == 16) {
		data = new char[subchunk2Size];

		short sample;
		for (int i = 0; i < subchunk2Size; i+=2) {
			// TODO: Shift left 8 instead
			sample = ( (unsigned char) rawData[i] ) * 256;
			sample = sample + (unsigned char) rawData[i+1];
			// TODO: Shift RIGHT 1 instead
			data[i/2] = sample;
		}
	}
	else {
		throw invalid_argument(bitsPerSample + " bits per sample is not supported.");
	}

	ifs.close();
}