#pragma once

#include "BoolLock.h"
#include "GCPointer.h"
#include <map>
#include <set>

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
		friend void reset();
		friend size_t live_object_count();
		friend size_t live_pointer_count();

		using OwnerPointerMap = std::multimap<const OwnerType*, gc_ptr_base*>;
		using BackingSet = std::set<gc_ptr_base::Backing*>;
		using MapIt = typename OwnerPointerMap::iterator;
		using Range = std::pair<MapIt, MapIt>;
		
		OwnerPointerMap owned;
		BackingSet backings;
		MapIt lastInsertion;
		bool isCollecting = false;
		
		void reset()
		{
			owned.clear();
			backings.clear();
		}
		
		void add(gc_ptr_base& ptr)
		{
			lastInsertion = owned.insert({ ptr.owner, &ptr });
			if (ptr.backing)
				backings.insert(ptr.backing);
		}
		
		void remove(gc_ptr_base& ptr)
		{
			map_remove(owned, ptr.owner, &ptr);
		}
		
		void removeBacking(gc_ptr_base::Backing& backing)
		{
			backings.erase(&backing);
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
			return owned.equal_range(owner);
		}
		
		void deleteUnmarked()
		{
			bool_lock lock(isCollecting);

			for (auto it = backings.begin(); it != backings.end(); )
			{
				gc_ptr_base::Backing& backing = **it;
				if (!backing.marked)
				{
					backing.deletePointee();
					it = backings.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		
		
		void unmarkAll()
		{
			for (auto entry : owned)
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
			return owned.equal_range(gc_ptr_base::cNoOwner);
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
			std::string result = std::string("gc_pool<") + typeid(T).name() + "> { size: " + std::to_string(owned.size());
			for (auto it = owned.begin(); it != owned.end(); ++it)
			{
				result += "\n\t{ " + it->first->to_string() + ", " + it->second->to_string() + " }";
			}
			result += owned.size() ? "\n}" : "}";
			
			return result;
		}
	};
}