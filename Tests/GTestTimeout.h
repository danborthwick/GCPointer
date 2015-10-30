#pragma once

#include <gmock/gmock.h>
#include <future>

namespace testing
{
	struct TimedTest {
		std::future<void> future;
	};
	
	::std::ostream& operator<<(::std::ostream& os, const TimedTest&)
	{
		return os << "Timed test";
	}

	
	MATCHER_P(RunsInLessThanMillis, timeoutMillis, "") {
		*result_listener << "took more than " << timeoutMillis << "ms to execute";
		TimedTest const& test = arg;
		bool timedOut = test.future.wait_for(std::chrono::milliseconds(timeoutMillis)) != std::future_status::timeout;
		
		// Complete test even if it times out, to avoid leaks etc.
		if (timedOut) {
			test.future.wait();
		}
		
		return timedOut;
	}

	TimedTest timedTest(std::function<void()> f) {
		return { std::async(std::launch::async, f) };
	}
}