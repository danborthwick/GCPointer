#include "Inheritance.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <numeric>

using namespace std;
using namespace testing;

using StringPtr = shared_ptr<string>;

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

TEST(SharedPointerTest, vectorInitialisation)
{
	using StringVector = vector<StringPtr>;
	StringVector v = { make_shared<string>("alpha"), make_shared<string>("beta") };
	
	ASSERT_THAT(*v[1], Eq("beta"));

	shared_ptr<StringVector> pv = make_shared<StringVector>(std::initializer_list<StringPtr> {
		make_shared<string>("alpha"), make_shared<string>("beta") });
	
	ASSERT_THAT(*(*pv)[1], Eq("beta"));
}

TEST(SharedPointerTest, vectorAccumulate)
{
	{
		shared_ptr<vector<StringPtr>> v = make_shared<vector<StringPtr>>();
		v->push_back(make_shared<string>("Once"));
		v->push_back(make_shared<string>("upon"));
		v->push_back(make_shared<string>("a"));
		v->push_back(make_shared<string>("time"));
		
		auto append = [] (StringPtr& accumulator, StringPtr& next) {
			if (!accumulator->empty())
				*accumulator += " ";
			
			*accumulator += *next;
			return accumulator;
		};
		
		shared_ptr<string> joined = accumulate(v->begin(), v->end(), make_shared<string>(), append);
		
		ASSERT_THAT(*joined, Eq("Once upon a time"));
	}
}
