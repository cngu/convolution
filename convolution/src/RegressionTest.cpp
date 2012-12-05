#include "RegressionTest.h"
#include <fstream>
#include <iostream>

using namespace std;

void RegressionTest::outputComparisonTest_Mono()
{
	ifstream time("output/mono.wav", ifstream::in | ifstream::binary);
	ifstream freq("output/monoFFT.wav", ifstream::in | ifstream::binary);

	time.seekg(44, ios::beg);
	freq.seekg(44, ios::beg);

	short timeBuff, freqBuff;

	time.read((char*)&timeBuff, 2);
	freq.read((char*)&freqBuff, 2);

	time.close();
	freq.close();

	cout << "Same?: " << timeBuff << " " << freqBuff << endl;
	system("pause");
}

void RegressionTest::outputComparisonTest_Stereo()
{
	ifstream time("output/mono.wav", ifstream::in | ifstream::binary);
	ifstream freq("output/monoFFT.wav", ifstream::in | ifstream::binary);

	time.seekg(44, ios::beg);
	freq.seekg(44, ios::beg);

	short timeBuff, freqBuff;

	time.read((char*)&timeBuff, 2);
	freq.read((char*)&freqBuff, 2);

	time.close();
	freq.close();

	cout << "Same?: " << timeBuff << " " << freqBuff << endl;
	system("pause");
}

void RegressionTest::runAllTests()
{
	outputComparisonTest_Mono();
	outputComparisonTest_Stereo();
}