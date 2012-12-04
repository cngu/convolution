#include <gtest\gtest.h>
#include "..\src\Convolver.h"

class Foo {

};

class FooTest : public ::testing::Test 
{
public:
	Convolver *c;

	FooTest() {
		// You can do set-up work for each test here.
		c = new Convolver();
	}

	virtual ~FooTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

	void t() {
		std::cout << "HELLO" << std::endl;
	}
};

TEST_F(FooTest, TestName1) 
{
	delete c;
	c = new Convolver;
	delete c;
	t();
	EXPECT_EQ(0.0, 0.00);
}

TEST(TESTCASE,TESTNAME) 
{
	EXPECT_FLOAT_EQ(1.5f, 1.5f);
}

TEST(MethodName,TestCaseNameLike_BoundaryCase) 
{
	EXIT_SUCCESS(1==1);
}
