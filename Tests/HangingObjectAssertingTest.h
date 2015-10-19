#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
	}
};