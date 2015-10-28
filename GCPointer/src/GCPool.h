#pragma once

#include "GCPointer.h"

namespace gc
{
	class gc_pool_base
	{
	public:
		static gc_pool_base sInstance;

		void collectGarbage()
		{
			unmarkAll();
			mark(unownedPointers());
			deleteUnmarked();
		}

	protected:
		using OwnerType = gc_ptr_base::OwnerType;
		friend class gc_ptr_base;
		friend size_t live_object_count();

		using OwnerPointerMap = std::multimap<const OwnerType*, gc_ptr_base*>;
		using MapIt = typename OwnerPointerMap::iterator;
		using Range = std::pair<MapIt, MapIt>;
		
		OwnerPointerMap pointers;
		
		void reset()
		{
			pointers.clear();
		}
		
		void add(gc_ptr_base& ptr)
		{
			pointers.insert({ ptr.owner, &ptr });
		}
		
		void remove(gc_ptr_base& ptr)
		{
			map_remove_value_if(pointers, &ptr);
		}
		
		void mark(Range range)
		{
			for (MapIt it = range.first; it != range.second; ++it)
			{
				gc_ptr_base& ptr = *it->second;
				if (ptr.backing)
				{
					if (ptr.backing->marked)
					{
						// Already encountered this part of the graph, terminate recursion
						break;
					}
					else
					{
						ptr.backing->marked = true;
					}
				}
				
				mark(childrenOf(ptr));
			}
		}
		
		Range childrenOf(gc_ptr_base& parent)
		{
			OwnerType* owner = (OwnerType*) parent.get_void();
			return pointers.equal_range(owner);
		}
		
		void deleteUnmarked()
		{
			for (auto it = pointers.begin(); it != pointers.end(); )
			{
				gc_ptr_base& ptr = *it->second;
				if (ptr.backing && !ptr.backing->marked)
				{
					auto& backing = *ptr.backing;
					nullifyPointersTo(backing.to);
					backing.deletePointee();
					
					// Iterator now invalid, start again
					it = pointers.begin();
				}
				else
					++it;
			}
		}
		
		void nullifyPointersTo(OwnerType* pointee)
		{
			for (auto entry : pointers)
			{
				gc_ptr_base& p = *entry.second;
				if (p.backing && (p.backing->to == pointee))
				{
					p.backing = nullptr;
				}
			}
		}
		
		void unmarkAll()
		{
			for (auto entry : pointers)
			{
				gc_ptr_base& p = *entry.second;
				if (p.backing)
				{
					p.backing->marked = false;
				}
			}
		}
		
		Range unownedPointers()
		{
			return pointers.equal_range(gc_ptr_base::cNoOwner);
		}
	};
	
	template<typename T>
	class gc_pool : public gc_pool_base
	{
	public:
		using Ptr = gc_ptr<T>;
		friend class gc_ptr<T>;
		
		static gc_pool<T>& instance()
		{
			return *(gc_pool<T>*)&sInstance;
		}
		
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
	};
}