#include "RegressionTest.h"
#include <fstream>
#include <iostream>

using namespace std;

/* Read 2 bytes at a time from file1 and file2 and compare. */
void RegressionTest::outputComparison(string file1, string file2)
{
	ifstream time(file1, ifstream::in | ifstream::binary);
	ifstream freq(file2, ifstream::in | ifstream::binary);

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
			break;
		}

		offset += 2;
	}

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