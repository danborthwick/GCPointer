#pragma once

#include <map>

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
void map_remove_value_if(std::multimap<Key, Value>& map, Value const& value)
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
void map_remove_value_if(std::multimap<Key, Value>& map, Value const& value,
						 typename std::multimap<Key, Value>::iterator& hint)
{
	if (hint->second == value)
	{
		hint = map.erase(hint, std::next(hint));
	}
	else
	{
		map_remove_value_if(map, value);
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