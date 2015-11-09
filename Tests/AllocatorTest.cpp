#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Inheritance.h"

using namespace std;
using namespace testing;

template <class Tp, size_t MaxSize>
struct SimpleAllocator {
	typedef Tp value_type;
	
	vector<value_type> values { MaxSize };
	size_t current { 0 };
	
	SimpleAllocator() {}
	
	~SimpleAllocator()
	{
		int i=0;
	}
	
	value_type* allocate(std::size_t n)
	{
		auto* result = &values[current];
		current += n;
		return result;
	}
	
	void deallocate(value_type* p, std::size_t n) {}
};

class AllocatorTest : public Test {};

TEST_F(AllocatorTest, CanAllocateWithFastAllocator)
{
	vector<Derived> pointers;
//	vector<Derived, SimpleAllocator<Derived, 10>> pointers;
	
	for (int i=0; i < 10; i++)
	{
		pointers.emplace_back();
	}
	
	ASSERT_THAT(pointers, Each(Property(&Derived::className, Eq("Derived"))));
}