#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "GCPointer.h"

using namespace gc;
using namespace std;
using namespace testing;

class Base
{
public:
	virtual string className() { return "Base"; }
};

class Derived : public Base
{
public:
	string className() override { return "Derived"; }
};

template<>
gc_pool<Base> gc_pool<Base>::sInstance {};

template<>
gc_pool<Derived> gc_pool<Derived>::sInstance {};


TEST(GCPointerCastingTest, CanBeDownCast)
{
	gc_ptr<Base> base { new Derived() };
	gc_ptr<Derived> derived = gc::dynamic_pointer_cast<Base, Derived>(base);
	
	//TODO: Don't fail due to not sharing impl!
	ASSERT_THAT(base->className(), Eq("Derived"));
	ASSERT_THAT(derived->className(), Eq("Derived"));
}

TEST(GCPointerCastingTest, CanBeUpCast)
{
//	gc_ptr<Derived> derived = make_gc(new Derived);
//	gc_ptr<Base> base = derived;
//	
//	ASSERT_THAT(base->className(), Eq("Derived"));
//	ASSERT_THAT(derived->className(), Eq("Derived"));
}