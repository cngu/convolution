#include <gtest\gtest.h>
#include "..\src\Convolve.h"

class Foo {

};

class FooTest : public ::testing::Test {
public:
	FooTest() {
		// You can do set-up work for each test here.
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

};

TEST_F(FooTest, TestName1) {
	Convolve *c = new Convolve();
	delete c;
	EXPECT_EQ(0.0, 0.00);
}

TEST(TESTCASE,TESTNAME) {
	EXPECT_FLOAT_EQ(1.5f, 1.5f);
}

TEST(MethodName,TestCaseNameLike_BoundaryCase) {
	EXIT_SUCCESS(1==1);
}
