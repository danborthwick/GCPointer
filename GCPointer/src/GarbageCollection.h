#pragma once

#include "GCPointer.h"
#include "GCPool.h"
#include <string>

namespace gc
{
	void collectGarbage();

	void reset();
	
	size_t live_object_count();
	size_t live_pointer_count();

	template <typename T, typename... ARGS>
	gc_ptr<T> make_gc(ARGS&&... args)
	{
		return gc_pool<T>::instance().makeUnowned(std::forward<ARGS>(args)...);
	}
	
	template <typename T, typename... ARGS>
	gc_ptr<T> make_owned_null_gc(gc_ptr_base::OwnerType* owner)
	{
		return gc_pool<T>::instance().makeOwnedNull(owner);
	}
	
	template <typename T, typename... ARGS>
	gc_ptr<T> make_owned_gc(gc_ptr_base::OwnerType* owner, ARGS&&... args)
	{
		return gc_pool<T>::instance().makeOwned(owner, std::forward<ARGS>(args)...);
	}
	
	template <typename Derived, typename Base>
	gc_ptr<Derived> dynamic_pointer_cast(gc_ptr<Base>& base)
	{
		return gc_pool<Base>::instance().template makeDynamicCast<Derived>(base);
	}
	
}