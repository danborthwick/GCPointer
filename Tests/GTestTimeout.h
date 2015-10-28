#pragma once

#include <future>

#define GTEST_TIMEOUT_BEGIN auto asyncFuture = std::async(std::launch::async, [this]()->void {

#define GTEST_TIMEOUT_END(X) return; }); \
EXPECT_TRUE(asyncFuture.wait_for(std::chrono::milliseconds(X)) != std::future_status::timeout);