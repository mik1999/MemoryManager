#pragma once

#include "AllocationStrategy.h"

template <class AllocStrategy>
class CAllocatedOn {
	void *operator new(size_t size);

	void operator delete(void *ptr);

	void *operator new[](size_t size);

	void operator delete[](void *ptr);
};

template<class AllocStrategy>
inline void * CAllocatedOn<AllocStrategy>::operator new(size_t size)
{
	return AllocStrategy::alloc(size);
}

template<class AllocStrategy>
inline void CAllocatedOn<AllocStrategy>::operator delete(void * ptr)
{
	AllocStrategy::free(ptr);
}

template<class AllocStrategy>
inline void * CAllocatedOn<AllocStrategy>::operator new[](size_t size)
{
	return AllocStrategy::alloc(size);
}

template<class AllocStrategy>
inline void CAllocatedOn<AllocStrategy>::operator delete[](void * ptr)
{
	AllocStrategy::free(ptr);
}

typedef CAllocatedOn<RuntimeHeapStategy> CRuntimeHeapAllocOn;
typedef CAllocatedOn<CurrentMemoryManagerStategy> CCurrentManagerAllocOn;
