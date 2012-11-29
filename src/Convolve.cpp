#include <iostream>

#include "Convolve.h"

void usage()
{
	std::cout << "Usage: convolve inputfile IRfile outputfile" << std::endl;
	std::cout << "All files must be one of the following formats: .wav .aiff .snd" << std::endl;   
}

// Returns true if arguments are valid. False otherwise.
bool checkArgs(int argc, const char* argv[])
{
	// TODO: Check file extensions for .wav, .aiff, .snd
	// Initially use rfind for '.' and then substr. and then comparison (yet another loop)
	// Code tuning: unrolling
	if (argc != 4) {
		usage();
		return false;
	}
	return true;
}

int main(int argc, const char* argv[])
{
	/* TODO: Uncomment when ready to pass in file inputs
	if (checkArgs(argc, argv)) {
		return -1;
	}*/



	return 0;		 	
}