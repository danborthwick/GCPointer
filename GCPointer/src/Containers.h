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
	map_remove_value_if(map, [&](Value const& candidate) {
		return candidate == value;
	});
}