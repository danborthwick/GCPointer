#include "GCPointer.h"

using namespace gc;

int Object::sInstanceCount { 0 };

const gc_ptr_base::OwnerType* gc_ptr_base::cNoOwner { nullptr };
