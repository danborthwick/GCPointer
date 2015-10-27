#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "GarbageCollection.h"

class HangingObjectAssertingTest : public testing::Test
{
public:
	void SetUp() override
	{
		gc::Object::resetInstanceCount();
	}
	
	void TearDown() override
	{
		ASSERT_THAT(gc::Object::instanceCount(), testing::Eq(0));
		ASSERT_THAT(gc::live_object_count(), testing::Eq(0));
	}
};