#ifndef SOUNDFILE_H
#define SOUNDFILE_H

#include <fstream>
#include <string>

class SoundFile
{
public:
	static SoundFile* create(char* filename);
	virtual void load(char* loadPath) = 0;
	static void save(char* savePath, int numChannels, int bitsPerSample, int sampleRate, short* data, int dataLen);

	void splitChannels(short* left, short* right, int len);
	void interleave(double* left, double* right, int len, double* output);
	void interleaveComplex(double* left, double* right, int len, double* output);

	virtual int getNumChannels() = 0;
	virtual int getSampleRate() = 0;
	virtual short getBitsPerSample() = 0;
	virtual int getDataSize() = 0;
	virtual short* getData() = 0;
	
	int getAbsMinValue();
	int getMaxValue();

	static size_t fwriteIntLSB(int data, FILE *stream);
	static size_t fwriteShortLSB(short int data, FILE *stream);
	static size_t fwriteIntMSB(int data, FILE *stream);
	static size_t fwriteShortMSB(short int data, FILE *stream);
	static std::string parseExtension(char* filename);

protected:
	void readIntChunk(std::ifstream& ifs, int& offset, const int& value);
	void readShortChunk(std::ifstream& ifs, int& offset, const short& value);
	void readUnsignedIntChunk(std::ifstream& ifs, int& offset, const unsigned int& value);
	
};

#endif