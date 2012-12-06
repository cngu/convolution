#include "Aiff.h"

using namespace std;

Aiff::Aiff(char* filePath)
{

}

Aiff::~Aiff()
{
	delete[] data;
}

int Aiff::getNumChannels()
{
	return numChannels;
}

int Aiff::getSampleRate()
{
	return sampleRate;
}

int Aiff::getDataSize()
{
	return dataSize;
}

short Aiff::getBitsPerSample()
{
	return 16;
}

short* Aiff::getData()
{
	return data;
}

void Aiff::load(char* loadPath)
{

}

void Aiff::saveHeader(FILE *outputFile, int numChannels, int dataLen, int sampleRate)
{
	int soundDataChunkSize = dataLen*2 + 8;

	// Common Chunk size + Common chunkID/Size + sound data chunk size + Sound Data chunkID/Size + Form Chunk type
	int formChunkSize = 18 + 8 + soundDataChunkSize + 8 + 4;
	
	// FORM AIFF Chunk
	fputs("FORM", outputFile);
	fwriteIntMSB(formChunkSize, outputFile);
	fputs("AIFF", outputFile);

	// Common Chunk
	fputs("COMM", outputFile);
	fwriteIntMSB(18, outputFile);
	fwriteShortMSB(numChannels, outputFile);
	fwriteIntMSB(dataLen, outputFile);
	fwriteShortMSB(16, outputFile);
	fwriteIntMSB(0, outputFile);
	fwriteIntMSB(sampleRate, outputFile);

	// Sound Chunk
	fputs("SSND", outputFile);
	fwriteIntMSB(soundDataChunkSize, outputFile);
	fwriteIntMSB(0, outputFile);
	fwriteIntMSB(0, outputFile);
}

void Aiff::saveData(FILE* outputFile, short data[], int len)
{
	for (int i = 0; i < len; i++) {
		SoundFile::fwriteShortMSB(data[i], outputFile);
	}
}

void Aiff::save(char* outputFile, int numChannels, int sampleRate, short* data, int dataLen)
{
	FILE *outputWaveFile = fopen(outputFile, "wb");
	saveHeader(outputWaveFile, numChannels, dataLen , sampleRate);
	saveData(outputWaveFile, data, dataLen);
	fclose(outputWaveFile);
}
