#include "GTestTimeout.h"
#include "HangingObjectAssertingTest.h"
#include "ObjectNetwork.h"

using namespace testing;

class PerformanceTest : public HangingObjectAssertingTest
{
};

TEST_F(PerformanceTest, largeNetworkIsGarbageCollectedQuickly)
{
	const int nodeCount = 5000;
	
	GTEST_TIMEOUT_BEGIN
	
	{
		ObjectNetwork network(nodeCount);
		ASSERT_THAT(InstanceCounted::instanceCount(), Eq(nodeCount));
	}
	
	ASSERT_THAT(InstanceCounted::instanceCount(), AllOf(Gt(0), Lt(nodeCount)));
	
	collectGarbage();
	
	GTEST_TIMEOUT_END(1000)
}