#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Containers.h"

using namespace std;
using namespace testing;

using intStringMap = multimap<int, string>;
const intStringMap cStart = {
	{ 4, "four" },
	{ 6, "six" },
	{ 6, "six2" },
	{ 7, "seven" },
	{ 8, "eight" }
};

const intStringMap cNoSixes = {
	{ 4, "four" },
	{ 7, "seven" },
	{ 8, "eight" }
};

TEST(MultimapTest, Erasing)
{
	using intStringMap = multimap<int, string>;
	intStringMap mm = cStart;

	for (auto it = mm.begin(); it != mm.end(); )
	{
		if (it->second.find("six") != string::npos)
		{
			it = mm.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	ASSERT_THAT(mm, Eq(cNoSixes));
	
}

TEST(MultimapTest, ErasingWithUtilFunc)
{
	using intStringMap = multimap<int, string>;
	intStringMap mm = cStart;

	map_remove_if_value(mm, [](string& s) {
		return s.find("six") != string::npos;
	});
	
	ASSERT_THAT(mm, Eq(cNoSixes));
	
}
