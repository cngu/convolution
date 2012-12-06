#include "RegressionTest.h"
#include <fstream>
#include <iostream>

using namespace std;

void RegressionTest::outputComparison(string file1, string file2)
{
	ifstream time(file1, ifstream::in | ifstream::binary);
	ifstream freq(file2, ifstream::in | ifstream::binary);

	// TODO: Read and compare header, 2 byte by 2 byte. Once you reach data, read entire data chunk based on subchunk2size
	int offset = 0;
	short timeSample, freqSample;
	while (! time.eof() && ! freq.eof() ) {
		time.seekg(offset, ios::beg);
		freq.seekg(offset, ios::beg);

		time.read((char*)&timeSample, sizeof(short));
		freq.read((char*)&freqSample, sizeof(short));

		if (! equalShorts(timeSample, freqSample)) {
			fail("RegressionTest", string("outputComparisonTest(" + file1 + "," + file2 + ")"));
			cout << timeSample << " " << freqSample << " " << offset << endl;
			system("pause");
		}

		offset += 2;
	}
	//
	time.seekg(44, ios::beg);
	freq.seekg(44, ios::beg);
	time.read((char*)&timeSample, sizeof(short));
	freq.read((char*)&freqSample, sizeof(short));

	cout << timeSample << " " << freqSample << endl;
	//
	time.close();
	freq.close();
}

void RegressionTest::runAllTests()
{
	cout << "Running RegressionTest..." << endl;
	outputComparison("output/mono.wav", "output/monoFFT.wav");
	outputComparison("output/stereo.wav", "output/stereoFFT.wav");
	cout << "RegressionTest completed.\n" << endl;
}