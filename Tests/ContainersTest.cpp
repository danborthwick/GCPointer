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

const intStringMap cOneSix = {
	{ 4, "four" },
	{ 6, "six" },
	{ 7, "seven" },
	{ 8, "eight" }
};

TEST(ContainersTest, MapErase)
{
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

TEST(ContainersTest, MapRemoveValue)
{
	intStringMap mm = cStart;

	map_remove_value(mm, string("six"));
	map_remove_value(mm, string("six2"));
	
	ASSERT_THAT(mm, Eq(cNoSixes));
}

TEST(ContainersTest, MapRemoveIf)
{
	intStringMap mm = cStart;
	
	map_remove_value_if(mm, [](string& s) {
		return s.find("six") != string::npos;
	});
	
	ASSERT_THAT(mm, Eq(cNoSixes));
}

TEST(ContainersTest, MapRemove)
{
	intStringMap mm = cStart;
	
	map_remove(mm, 6, string("six2"));
	
	ASSERT_THAT(mm, Eq(cOneSix));
}

using StringSet = unordered_multiset<string>;
const StringSet cEvenStringSet = {
	"two", "four", "six", "eight"
};

TEST(ContainersTest, SetRemove)
{
	StringSet set = cEvenStringSet;
	set_remove_value_with_rehash(set, string("four"));
	ASSERT_THAT(set, Eq(StringSet { "two", "six", "eight" }));
}

struct StringPointerHash {
	size_t operator() (string* const& s) const
	{
		return std::hash<string>()(*s);
	}
};
using StringPointerSet = unordered_multiset<string*, StringPointerHash>;

TEST(ContainersTest, SetRemoveAfterHashInvalidated)
{
	string immutables[] = { "immutable 1", "immutable 2" };
	string mutableString = "mutableBefore";
	
	StringPointerSet set = { &mutableString, &immutables[0], &immutables[1] };
	
	mutableString = "mutableAfter"; // Changes expected hash
	
	// Should do nothing as hash has changed
	set_remove_value(set, &mutableString);
	ASSERT_THAT(set, SizeIs(3));
	
	set_remove_value_with_rehash(set, &mutableString);
	
	ASSERT_THAT(set, Eq(StringPointerSet { &immutables[0], &immutables[1] }));
}