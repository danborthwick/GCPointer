#include "gtest/gtest.h"
#include "GCObject.h"

int main(int argc, char * argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

int gc::Object::sInstanceCount { 0 };
