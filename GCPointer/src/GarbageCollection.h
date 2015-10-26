#pragma once

#include "GCPointer.h"
#include "GCPool.h"
#include <string>

namespace gc
{
	template <typename T, typename... ARGS>
	gc_ptr<T> make_gc(ARGS&&... args)
	{
		return gc_pool<T>::sInstance.makeUnowned(std::forward<ARGS>(args)...);
	}
	
	template <typename T, typename... ARGS>
	gc_ptr<T> make_owned_null_gc(gc_ptr_base::OwnerType* owner)
	{
		return gc_pool<T>::sInstance.makeOwnedNull(owner);
	}
	
	template <typename T, typename... ARGS>
	gc_ptr<T> make_owned_gc(gc_ptr_base::OwnerType* owner, ARGS&&... args)
	{
		return gc_pool<T>::sInstance.makeOwned(owner, std::forward<ARGS>(args)...);
	}
	
	template <typename T>
	void collectGarbage()
	{
		gc_pool<T>::sInstance.collectGarbage();
	}
	
	template <class Base, class Derived>
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
	
	template<typename T>
	std::string to_string(T const* t)
	{
		return t ? t->to_string() : "null";
	}
	
//	template<>
//	std::string to_stringy(void const*)
//	{
//		return "void";
//	}
}