#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "GarbageCollection.h"
#include "TestObjects.h"

class HangingObjectAssertingTest : public testing::Test
{
public:
	void SetUp() override
	{
		InstanceCounted::resetInstanceCount();
		gc::reset();
	}
	
	void TearDown() override
	{
		ASSERT_THAT(InstanceCounted::instanceCount(), testing::Eq(0));
		ASSERT_THAT(gc::live_pointer_count(), testing::Eq(0));
		ASSERT_THAT(gc::live_object_count(), testing::Eq(0));
	}
};