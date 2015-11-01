#include "GTestTimeout.h"
#include "HangingObjectAssertingTest.h"
#include "ObjectNetwork.h"

using namespace testing;

class PerformanceTest : public HangingObjectAssertingTest
{
};

TEST_F(PerformanceTest, largeNetworkIsGarbageCollectedQuickly)
{
	const int nodeCount = 50000;
	
	auto test = timedTest([] {
		{
			ObjectNetwork network(nodeCount);
			ASSERT_THAT(InstanceCounted::instanceCount(), Eq(nodeCount));
		}
		
		ASSERT_THAT(InstanceCounted::instanceCount(), AllOf(Gt(0), Lt(nodeCount)));
		
		collectGarbage();
	});
	
	ASSERT_THAT(test, RunsInLessThanMillis(300));
}