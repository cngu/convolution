#ifndef AIFF_H
#define AIFF_H

#include "SoundFile.h"

class Aiff : public SoundFile 
{
public:
	Aiff(char* wavePath);
	~Aiff();

	void load(char* loadPath);

	int getNumChannels();
	int getSampleRate();
	short getBitsPerSample();
	int getDataSize();
	short* getData();
	
};

#endif