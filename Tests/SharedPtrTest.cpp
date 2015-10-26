#include "Inheritance.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

using namespace std;
using namespace testing;

TEST(SharedPtrTest, BasePointerCanBeMade)
{
	shared_ptr<Base> p = make_shared<Derived>();
	ASSERT_THAT(p->className(), Eq("Derived"));
}

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

TEST(SharedPtrTest, UpCastPointerToNonBaseGivesCompileError)
{
	shared_ptr<NotDerivedFromBase> notDerived = make_shared<NotDerivedFromBase>();
// Should not compile	shared_ptr<Base> base = dynamic_pointer_cast<Base>(notDerived);
}
