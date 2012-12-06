#include "Snd.h"

using namespace std;

Snd::Snd(char* filePath)
{
	load(filePath);
}

Snd::~Snd()
{
	delete[] data;
}

int Snd::getNumChannels()
{
	return channels;
}

int Snd::getSampleRate()
{
	return sampleRate;
}

int Snd::getDataSize()
{
	return dataSize;
}

short Snd::getBitsPerSample()
{
	return 16;	// Fixed since we're assuming 16-bit linear PCM for this assignment (see testtone.c on BB)
}

short* Snd::getData()
{
	return data;
}

void Snd::load(char* loadPath)
{
	ifstream ifs(loadPath, ios::in);
	if (ifs.fail()) {
		throw invalid_argument("Snd::load: " + string(loadPath) + " could not be opened.");
	}

	/* Read Header */
	int offset = 4;
	readUnsignedIntChunk(ifs, offset, dataOffset);
	readUnsignedIntChunk(ifs, offset, dataSizeInBytes);
	offset += 4;							// skip data format, fixed at 16-bit linear PCM
	readUnsignedIntChunk(ifs, offset, sampleRate);
	readUnsignedIntChunk(ifs, offset, channels);

	/* Read Data */
	char* rawData = new char[dataSizeInBytes];
	ifs.seekg(dataOffset, ios::beg);
	ifs.read(rawData, dataSizeInBytes);

	// TODO: Shift RIGHT 1 instead
	dataSize = dataSizeInBytes/2;
	data = new short[dataSize];

	short sample;
	for (int i = 0; i < dataSizeInBytes; i+=2) {
		// TODO: Shift left 8 instead
		sample = (short) ( (unsigned char) rawData[i] );
		sample += (short) ( (unsigned char) rawData[i+1]) * 256;
		// TODO: Shift RIGHT 1 instead
		data[i/2] = sample;
	}

	ifs.close();
	delete[] rawData;
}

void Snd::saveHeader(FILE *outputFile, unsigned int theDataOffset, unsigned int theDataSize, unsigned int theSampleRate, unsigned int theChannels)
{
	// Magic
	fputs(".snd", outputFile);

	// Data Offset
	SoundFile::fwriteIntMSB(theDataOffset, outputFile);

	// Data Size (in bytes)
	SoundFile::fwriteIntMSB(theDataSize, outputFile);

	// Encoding = 3 = 16-bit linear PCM
	SoundFile::fwriteIntMSB(3, outputFile);

	// Sample Rate
	SoundFile::fwriteIntMSB(theSampleRate, outputFile);

	// Channels
	SoundFile::fwriteIntMSB(theChannels, outputFile);

	// Write optional annotation
	theDataOffset -= 24;
	while (theDataOffset > 0) {
		SoundFile::fwriteIntMSB(0, outputFile);
		theDataOffset -= 4;
	}
}

void Snd::saveData(FILE* outputFile, short data[], int len)
{
	for (int i = 0; i < len; i++) {
		SoundFile::fwriteShortMSB(data[i], outputFile);
	}
}

//(char* outputFile, int channels, int numberSamples, int bitsPerSample, double sampleRate, short data[], int dataLen) 
void Snd::save(char* outputFile, unsigned int theDataOffset, unsigned int theSampleRate, unsigned int theChannels, short data[], unsigned int dataLen)
{
	FILE *outputWaveFile = fopen(outputFile, "wb");
	int dataChunkSize = 2 * dataLen;
	saveHeader(outputWaveFile, theDataOffset, dataChunkSize , theSampleRate, theChannels);
	saveData(outputWaveFile, data, dataLen);
	fclose(outputWaveFile);
}