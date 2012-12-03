#ifndef SOUNDFILE_H
#define SOUNDFILE_H

#include <fstream>
#include <string>

class SoundFile
{
public:
	void readIntChunk(std::ifstream& ifs, int& offset, const int& value);
	void readShortChunk(std::ifstream& ifs, int& offset, const short& value);

	static SoundFile* create(char* filename);
	virtual void load(char* loadPath) = 0;

	virtual int getNumChannels() = 0;
	virtual int getDataSize() = 0;
	virtual short getBitsPerSample() = 0;
	virtual short* getData() = 0;

	static std::string parseExtension(char* filename);

	
};

#endif