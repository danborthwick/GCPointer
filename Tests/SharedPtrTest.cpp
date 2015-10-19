#include "Inheritance.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

using namespace std;
using namespace testing;

TEST(SharedPtrTest, DownCastPointerToBase)
{
	shared_ptr<Base> base { new Derived };
	shared_ptr<Derived> derived = dynamic_pointer_cast<Derived>(base);
	
	ASSERT_THAT(base->className(), Eq("Derived"));
	ASSERT_THAT(derived->className(), Eq("Derived"));
}

TEST(SharedPtrTest, UpCastPointerToDerived)
{
	shared_ptr<Derived> derived(new Derived);
	shared_ptr<Base> base = derived;
	
	ASSERT_THAT(base->className(), Eq("Derived"));
	ASSERT_THAT(derived->className(), Eq("Derived"));
}