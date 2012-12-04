#include "Snd.h"

using namespace std;

Snd::Snd(char* filePath)
{

}

Snd::~Snd()
{

}

int Snd::getNumChannels()
{
	return -1;
}

int Snd::getSampleRate()
{
	return -1;
}

int Snd::getDataSize()
{
	return -1;
}

short Snd::getBitsPerSample()
{
	return -1;
}

short* Snd::getData()
{
	return nullptr;
}

void Snd::load(char* loadPath)
{

}

