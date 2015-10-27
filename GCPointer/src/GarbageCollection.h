#pragma once

#include "GCPointer.h"
#include "GCPool.h"
#include <string>

namespace gc
{
	void collectGarbage();

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
	
	template <class Derived, class Base>
	gc_ptr<Derived> dynamic_pointer_cast(gc_ptr<Base>& base)
	{
		using DerivedImplClass = typename gc_ptr<Derived>::impl_class;
		gc_ptr<Derived> derived;
		
		if (dynamic_cast<Derived*>(base.get()))
		{
			derived.owner = base.owner;
			derived.impl = (DerivedImplClass*)((void*)base.impl);
		}
		return derived;
	}
}