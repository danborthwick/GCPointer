#pragma once

#include "GCPointer.h"

namespace gc
{
	class gc_pool_base
	{
	public:
		using OwnerType = gc_ptr_base::OwnerType;
		friend class gc_ptr_base;
	};
	
	template<typename T>
	class gc_pool : public gc_pool_base
	{
	public:
		using Ptr = gc_ptr<T>;
		friend class gc_ptr<T>;
		
		static gc_pool<T> sInstance;
		
		template<typename... ARGS>
		Ptr makeUnowned(ARGS... args)
		{
			return makeOwned(Ptr::cNoOwner, std::forward<ARGS>(args)...);
		}
		
		Ptr makeUnownedFromInstance(T* instance)
		{
			Ptr p { Ptr::cNoOwner, instance };
			add(p);
			return p;
		}
		
		template<typename... ARGS>
		Ptr makeOwned(const OwnerType* owner, ARGS... args)
		{
			Ptr p(owner, new T(std::forward<ARGS>(args)...));
			add(p);
			return p;
		}
		
		Ptr makeOwnedNull(OwnerType* owner)
		{
			Ptr p(owner, nullptr);
			add(p);
			return p;
		}
		
		void reset()
		{
			pointers.clear();
		}
		
		void collectGarbage()
		{
			unmarkAll();
			mark(unownedPointers());
			deleteUnmarked();
		}
		
		std::string to_string() const
		{
			std::string result = std::string("gc_pool<") + typeid(T).name() + "> { size: " + std::to_string(pointers.size());
			for (auto it = pointers.begin(); it != pointers.end(); ++it)
			{
				result += "\n\t{ " + it->first->to_string() + ", " + it->second->to_string() + " }";
			}
			result += pointers.size() ? "\n}" : "}";
			
			return result;
		}
		
	private:
		using OwnerPointerMap = std::multimap<const OwnerType*, Ptr*>;
		using MapIt = typename OwnerPointerMap::iterator;
		using Range = std::pair<MapIt, MapIt>;
		
		OwnerPointerMap pointers;
		
		void add(Ptr& ptr)
		{
			pointers.insert({ ptr.owner, &ptr });
		}
		
		void remove(Ptr& ptr)
		{
			map_remove_if_value(pointers, [&](Ptr* candidate) {
				return candidate == &ptr;
			});
		}
		
		void mark(Range range)
		{
			for (MapIt it = range.first; it != range.second; ++it)
			{
				Ptr& ptr = *it->second;
				if (ptr.impl)
				{
					if (ptr.impl->marked)
					{
						// Already encountered this part of the graph, terminate recursion
						break;
					}
					else
						ptr.impl->marked = true;
				}
				
				Range children = pointers.equal_range(ptr.get());
				mark(children);
			}
		}
		
		void deleteUnmarked()
		{
			for (auto it = pointers.begin(); it != pointers.end(); )
			{
				Ptr& ptr = *it->second;
				if (ptr.impl && !ptr.impl->marked)
				{
					Object* pointee = ptr.get();
					nullifyPointersTo(*ptr);
					
					// TODO: Need to prevent this invalidating iterator
					delete pointee;
					
					// Iterator now invalid, start again
					it = pointers.begin();
				}
				else
					++it;
			}
		}
		
		void nullifyPointersTo(OwnerType& pointee)
		{
			for (auto entry : pointers)
			{
				Ptr& p = *entry.second;
				if (p.impl && (p.impl->to == &pointee))
				{
					p.impl = nullptr;
				}
			}
		}
		
		void unmarkAll()
		{
			for (auto entry : pointers)
			{
				Ptr& p = *entry.second;
				if (p.impl)
				{
					p.impl->marked = false;
				}
			}
		}
		
		Range unownedPointers()
		{
			return pointers.equal_range(Ptr::cNoOwner);
		}
	};
}