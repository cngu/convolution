#ifndef WAVE_H
#define WAVE_H

#include <memory>

class Wave 
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

	short min;
	short max;

public:
	Wave(char* wavePath);
	~Wave();

	void loadWave(char* loadPath);
};



#endif