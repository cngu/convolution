#include <iostream>
#include <stdexcept>
#include <string>

#include "SoundFile.h"
#include "Wave.h"
#include "Aiff.h"
#include "Snd.h"

using namespace std;

void SoundFile::readIntChunk(ifstream& ifs, int& offset, const int& value)
{
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &value, sizeof(value) );
	offset += sizeof(value);
}

void SoundFile::readShortChunk(ifstream& ifs, int& offset, const short& value)
{
	ifs.seekg(offset, ios::beg);
	ifs.read( (char*) &value, sizeof(value) );
	offset += sizeof(value);
}

/* Written By Leonard Manzara */
size_t SoundFile::fwriteIntLSB(int data, FILE *stream)
{
	unsigned char array[4];

    array[3] = (unsigned char)((data >> 24) & 0xFF);
    array[2] = (unsigned char)((data >> 16) & 0xFF);
    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    return fwrite(array, sizeof(unsigned char), 4, stream);
}

/* Written By Leonard Manzara */
size_t SoundFile::fwriteShortLSB(short int data, FILE *stream)
{
	unsigned char array[2];

    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    return fwrite(array, sizeof(unsigned char), 2, stream);
}

SoundFile* SoundFile::create(char* filename)
{
	string extension = SoundFile::parseExtension(filename);

	try {
		if (extension.compare(".wav") == 0) {
			return new Wave(filename);
		}
		else if (extension.compare(".aiff") == 0) {
			return new Aiff(filename);
		}
		else if (extension.compare(".snd") == 0) {
			return new Snd(filename);
		}
		else {
			throw invalid_argument("SoundFile::create - Invalid extension.");
		}
	}
	catch (invalid_argument& ia) {
		cout << ia.what() << endl;
		return nullptr;
	}
}

void SoundFile::splitChannels(short* left, short* right, int len)
{
	for (int i = 0; i < len; i++) {
		left[i] = getData()[i*2];
		right[i] = getData()[i*2 + 1];
	}
}

void SoundFile::interleave(double* left, double* right, int len, double* output)
{
	for (int i = 0; i < len; i++) {
		output[i*2] = left[i];
		output[i*2+1] = right[i];
	}
}

void SoundFile::interleaveComplex(double* left, double* right, int len, double* output)
{
	// TODO: calculate i*4 and i*2 only once per iteration. Use bit shifts too. Same with normal interlave above
	for (int i = 0; i < len/2; i++) {
		output[i*4] = left[i*2];
		output[i*4+1] = left[i*2+1];
		output[i*4+2] = right[i*2];
		output[i*4+3] = right[i*2+1];
	}
}

string SoundFile::parseExtension(char* filename) 
{
	string file(filename);
	return file.substr(file.rfind("."));
}