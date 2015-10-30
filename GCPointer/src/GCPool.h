#pragma once

#include "GCPointer.h"
#include <map>
#include <unordered_set>

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
		friend size_t live_pointer_count();

		using PointerSet = std::unordered_set<gc_ptr_base*>;
		using OwnerPointerMap = std::multimap<const OwnerType*, gc_ptr_base*>;
		using MapIt = typename OwnerPointerMap::iterator;
		using Range = std::pair<MapIt, MapIt>;
		
		PointerSet allPointers;
		OwnerPointerMap ownedPointers;
		MapIt lastInsertion;
		
		void reset()
		{
			ownedPointers.clear();
		}
		
		void add(gc_ptr_base& ptr)
		{
			lastInsertion = ownedPointers.insert({ ptr.owner, &ptr });
			allPointers.insert(&ptr);
		}
		
		void remove(gc_ptr_base& ptr)
		{
			map_remove(ownedPointers, ptr.owner, &ptr);
			allPointers.erase(&ptr);
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
			return ownedPointers.equal_range(owner);
		}
		
		void deleteUnmarked()
		{
			for (auto it = allPointers.begin(); it != allPointers.end(); )
			{
				gc_ptr_base& ptr = **it;
				if (ptr.backing && !ptr.backing->marked)
				{
					auto* backing = ptr.backing;
					nullifyPointersTo(backing->to);
					backing->deletePointee();
					delete backing;
					
					// Iterator now invalid, start again
					// TODO: One pass
					it = allPointers.begin();
				}
				else
					++it;
			}
		}
		
		void nullifyPointersTo(void* pointee)
		{
			for (auto it : allPointers)
			{
				gc_ptr_base& ptr = *it;
				if (ptr.backing && (ptr.backing->to == pointee))
				{
					ptr.backing = nullptr;
				}
			}
		}
		
		void unmarkAll()
		{
			for (auto it : allPointers)
			{
				gc_ptr_base& ptr = *it;
				if (ptr.backing)
				{
					ptr.backing->marked = false;
				}
			}
		}
		
		Range unownedPointers()
		{
			return ownedPointers.equal_range(gc_ptr_base::cNoOwner);
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
		
		template <typename Derived>
		gc_ptr<Derived> makeDynamicCast(gc_ptr<T>& base)
		{
			gc_ptr<Derived> p = base.template make_dynamic_cast<Derived, T>();
			add(p);
			return p;
		}
		
		std::string to_string() const
		{
			std::string result = std::string("gc_pool<") + typeid(T).name() + "> { size: " + std::to_string(ownedPointers.size());
			for (auto it = ownedPointers.begin(); it != ownedPointers.end(); ++it)
			{
				result += "\n\t{ " + it->first->to_string() + ", " + it->second->to_string() + " }";
			}
			result += ownedPointers.size() ? "\n}" : "}";
			
			return result;
		}
	};
}