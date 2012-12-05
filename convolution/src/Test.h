#ifndef TEST_H
#define TEST_H

class Test 
{
public:
	void pass(char* className, char* method);
	void fail(char* className, char* method, char* message);

	bool equalDoubles(double a, double b);
};

#endif