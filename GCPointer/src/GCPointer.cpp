#include "GCPointer.h"
#include "GCPool.h"

using namespace gc;

int Object::sInstanceCount { 0 };

const gc_ptr_base::OwnerType* gc_ptr_base::cNoOwner { nullptr };

gc_pool_base gc_pool_base::sInstance {};