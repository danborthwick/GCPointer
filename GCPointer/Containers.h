#pragma once

template<typename Key, typename Value, class Predicate>
void map_remove_if_value(std::multimap<Key, Value>& map, Predicate predicate)
{
	for (auto it = map.begin(); it != map.end(); )
	{
		if (predicate(it->second))
			it = map.erase(it);
		else
			++it;
	}
}