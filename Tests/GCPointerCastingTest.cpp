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

TEST(GCPointerCastingTest, CanBeDownCast)
{
	gc_ptr<Base> base = make_gc<Base>(new Derived());
	gc_ptr<Derived> derived = gc::dynamic_pointer_cast<Base, Derived>(base);
	
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