#ifndef WAVE_H
#define WAVE_H

class Wave 
{
public:
	static const int chunkID     = 0x46464952;  // "FFIR"
	static const int format      = 0x45564157;  // "EVAW"
	static const int subChunk1ID = 0x20746d66;  // " tmf"
	static const int subChunk2ID = 0x61746164;  // "atad"

private:
	int chunkSize;
	int subchunk1ID;
	int subchunk1Size;
	short audioFormat;
	short numChannels;
	int sampleRate;
	int byteRate;
	short blockAlign;
	short bitsPerSample;
	int subchunk2Size;
	char* data;

public:
	Wave(char* wavePath);
	~Wave();

	void readWave(char* wavePath);
};



#endif