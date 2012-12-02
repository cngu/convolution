#include <iostream>

#include "Wave.h"
#include "gtest\gtest.h"

using namespace std;

void usage()
{
	std::cout << "Usage: convolve inputfile IRfile outputfile" << std::endl;
	std::cout << "All files must be one of the following formats: .wav .aiff .snd" << std::endl;   
}

// Returns true if arguments are valid. False otherwise.
bool checkArgs(int argc, char* argv[])
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

unique_ptr<Wave> createWave(char* filename)
{
	try {
		return unique_ptr<Wave>(new Wave(filename));
	}
	catch (invalid_argument& ia) {
		cout << ia.what() << endl;
		return nullptr;
	}
}

int main(int argc, char* argv[])
{
	/* Ensure dry, IR, and output file names are provided */
	if (! checkArgs(argc, argv))
		return 1;

	unique_ptr<Wave> dryRecording = createWave(argv[1]);
	unique_ptr<Wave> impulseResponse = createWave(argv[2]);

	/* End program if any input file is corrupted or nonexistant */
	// TODO: do this check in between createWave's above. If first 
	// is corrupted, no point in creating Wave objects for the rest
	if (dryRecording == nullptr || impulseResponse == nullptr)
		return 1;

	

	/* Run tests */
	//::testing::InitGoogleTest(&argc, argv);
	//RUN_ALL_TESTS();

	dryRecording = nullptr;

	return 0;
}