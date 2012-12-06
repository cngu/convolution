#ifndef REGRESSIONTEST_H
#define REGRESSIONTEST_H

#include <string>

#include "Test.h"

class RegressionTest : public Test
{
public:
	static void outputComparison(std::string file1, std::string file2);
	static void runAllTests();
};

#endif