#ifndef AIFF_H
#define AIFF_H

#include "SoundFile.h"

// Reference: http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/AIFF/Docs/AIFF-1.3.pdf
//			  http://muratnkonar.com/aiff/index.html
//			  

class Aiff : public SoundFile 
{
public:
	// Common Chunk
	short numChannels;
	int numSampleFrames;
	short sampleSize;
	int sampleRate;

	// Sound Data Chunk
	int soundDataChunkSize;
	short* data;
	int dataSize;

public:
	Aiff(char* wavePath);
	~Aiff();

	void load(char* loadPath);

	int getNumChannels();
	int getSampleRate();
	short getBitsPerSample();
	int getDataSize();
	short* getData();
	
	static void saveHeader(FILE *outputFile, int numChannels, int dataLen, int sampleRate);
	static void saveData(FILE* outputFile, short data[], int len);
	static void save(char* outputFile, int numChannels, int sampleRate, short* data, int dataLen);
};

#endif