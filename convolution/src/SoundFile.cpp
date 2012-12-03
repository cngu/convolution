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

string SoundFile::parseExtension(char* filename) 
{
	string file(filename);
	return file.substr(file.rfind("."));
}