#include "GarbageCollection.h"

using namespace gc;

const gc_ptr_base::OwnerType* gc_ptr_base::cNoOwner { nullptr };

gc_pool_base gc_pool_base::sInstance {};

void gc::collectGarbage()
{
	gc_pool_base::sInstance.collectGarbage();
}

size_t gc::live_pointer_count()
{
	auto& pool = gc_pool_base::sInstance;
	return pool.allPointers.size();
}