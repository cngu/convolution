#ifndef REGRESSIONTEST_H
#define REGRESSIONTEST_H

#include "Test.h"

class RegressionTest : Test
{
public:
	static void outputComparisonTest_Mono();
	static void outputComparisonTest_Stereo();
	static void runAllTests();
};

#endif