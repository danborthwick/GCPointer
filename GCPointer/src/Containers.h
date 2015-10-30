#pragma once

#include <map>
#include <unordered_set>

template<typename Key, typename Value, class Predicate>
void map_remove_value_if(std::multimap<Key, Value>& map, Predicate predicate)
{
	for (auto it = map.begin(); it != map.end(); )
	{
		if (predicate(it->second))
			it = map.erase(it);
		else
			++it;
	}
}

template<typename Key, typename Value>
void map_remove_value(std::multimap<Key, Value>& map, Value const& value)
{
	for (auto it = map.begin(); it != map.end(); )
	{
		if (it->second == value)
			it = map.erase(it);
		else
			++it;
	}
}

template<typename Key, typename Value>
void map_remove_value(std::multimap<Key, Value>& map, Value const& value,
						 typename std::multimap<Key, Value>::iterator& hint)
{
	if (hint->second == value)
	{
		hint = map.erase(hint, std::next(hint));
	}
	else
	{
		map_remove_value(map, value);
	}
}

template<typename Key, typename Value>
void map_remove(std::multimap<Key, Value>& map, Key const& key, Value const& value)
{
	auto r = map.equal_range(key);
	for (auto it = r.first; it != r.second; ++it)
	{
		if (it->second == value)
		{
			map.erase(it);
			break;
		}
	}
}

template<typename Value, typename Hash>
void set_remove_value(std::unordered_multiset<Value, Hash>& set, Value const& value)
{
	set.erase(value);
}

template<typename Value, typename Hash>
void set_remove_value_with_rehash(std::unordered_multiset<Value, Hash>& set, Value const& value)
{
	auto position = set.find(value);
	
	if (position == set.end())
	{
		set.rehash(0);
		position = set.find(value);
	}
	
	set.erase(value);
}
