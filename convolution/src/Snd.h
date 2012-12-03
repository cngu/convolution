#ifndef SND_H
#define SND_H

#include "SoundFile.h"

class Snd : public SoundFile 
{
public:
	Snd(char* wavePath);
	~Snd();

	void load(char* loadPath);

	int getNumChannels();
	int getDataSize();
	short getBitsPerSample();
	short* getData();
	
};

#endif