#ifndef WAVE_H
#define WAVE_H

#include "SoundFile.h"

class Wave : public SoundFile
{
public:
	static const int chunkID     = 0x46464952;  // "FFIR"
	static const int format      = 0x45564157;  // "EVAW"
	static const int subchunk1ID = 0x20746d66;  // " tmf"
	static const int subchunk2ID = 0x61746164;  // "atad"

	int chunkSize;
	int subchunk1Size;
	short audioFormat;
	short numChannels;
	int sampleRate;
	int byteRate;
	short blockAlign;
	short bitsPerSample;
	int subchunk2Size;

	int dataSize;
	short* data;

public:
	Wave(char* filePath);
	~Wave();

	int getNumChannels();
	int getSampleRate();
	short getBitsPerSample();
	int getDataSize();
	short* getData();

	void load(char* loadPath);
	static void save(char* outputFile, int channels, int numberSamples, int bitsPerSample, 
				     double sampleRate, short data[], int dataLen);
	
	void splitChannels(short* left, short* right, int len);

private:
	static void saveHeader(FILE *outputFile, int channels, int numberSamples, int bitsPerSample, 
					       double sampleRate);
	static void saveData(FILE* outputFile, short data[], int len);



};



#endif