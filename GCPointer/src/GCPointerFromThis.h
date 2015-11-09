#pragma once

#include "GCPointer.h"

namespace gc
{
	template<typename T>
	class enable_gc_pointer_from_this
	{
	public:
		gc_ptr<T> gc_pointer_from_this()
		{
			return gc_pool<T>::instance().gcPointerFromRawPointer((T*) this);
		}
	};
}