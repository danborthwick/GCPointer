#include "GarbageCollection.h"
#include "HangingObjectAssertingTest.h"
#include "Inheritance.h"

using namespace gc;
using namespace std;
using namespace testing;

class GCPointerCastingTest : public HangingObjectAssertingTest
{
};

TEST_F(GCPointerCastingTest, BasePointerCanBeMade)
{
	gc_ptr<Base> p = make_gc<Derived>();
	ASSERT_THAT(p->className(), Eq("Derived"));
}


TEST_F(GCPointerCastingTest, DownCastPointerToBase)
{
	gc_ptr<Base> base { new Derived() };
	gc_ptr<Derived> derived = gc::dynamic_pointer_cast<Derived>(base);
	
	ASSERT_THAT(base->className(), Eq("Derived"));
	ASSERT_THAT(derived->className(), Eq("Derived"));
}

TEST_F(GCPointerCastingTest, DownCastPointerToNonBaseGivesNull)
{
	gc_ptr<Base> base { new Base() };
	gc_ptr<NotDerivedFromBase> derived = gc::dynamic_pointer_cast<NotDerivedFromBase>(base);

	ASSERT_THAT(derived.get(), IsNull());
}

TEST_F(GCPointerCastingTest, UpCastPointerToDerived)
{
	gc_ptr<Derived> derived = make_gc<Derived>();
	gc_ptr<Base> base = gc::dynamic_pointer_cast<Base>(derived);

	ASSERT_THAT(base->className(), Eq("Derived"));
	ASSERT_THAT(derived->className(), Eq("Derived"));
}

TEST_F(GCPointerCastingTest, UpCastPointerToNonBaseGivesCompileError)
{
	gc_ptr<NotDerivedFromBase> notDerived = make_gc<NotDerivedFromBase>();
// Compile error	gc_ptr<Base> base = gc::dynamic_pointer_cast<Base>(notDerived);
}

//TODO: Test dynamic casts are garbage collected