#ifndef SND_H
#define SND_H

#include "SoundFile.h"

// Reference: http://sox.sourceforge.net/AudioFormats-11.html
//			  http://en.wikipedia.org/wiki/Au_file_format
//			  http://www-mmsp.ece.mcgill.ca/documents/audioformats/AU/AU.html

class Snd : public SoundFile 
{
public:
	unsigned int dataOffset;
	unsigned int dataSizeInBytes;
	unsigned int sampleRate;
	unsigned int channels;
	
	short* data;
	unsigned int dataSize;

public:
	Snd(char* filePath);
	~Snd();

	void load(char* loadPath);

	int getNumChannels();
	int getSampleRate();
	short getBitsPerSample();
	int getDataSize();
	short* getData();
	
	static void saveHeader(FILE *outputFile, unsigned int theDataOffset, unsigned int theDataSize, unsigned int theSampleRate, unsigned int theChannels);
	static void saveData(FILE* outputFile, short data[], int len);
	static void save(char* outputFile, unsigned int theDataOffset, unsigned int theSampleRate, unsigned int theChannels, short data[], unsigned int dataLen);
};

#endif