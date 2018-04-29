#pragma once

#include "AllocationStrategy.h"

template <class AllocStrategy>
class CAllocatedOn {
public:
	void *operator new(size_t size);

	void operator delete(void *ptr);

	void *operator new[](size_t size);

	void operator delete[](void *ptr);

	void *operator new(size_t size, void *p);

	void operator delete  (void* ptr, void* place);
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

template<class AllocStrategy>
inline void * CAllocatedOn<AllocStrategy>::operator new(size_t size, void * p)
{
	return p;
}

template<class AllocStrategy>
inline void CAllocatedOn<AllocStrategy>::operator delete(void * ptr, void * place)
{
	//do nothing
}

typedef CAllocatedOn<RuntimeHeapStategy> CRuntimeHeapAllocOn;
typedef CAllocatedOn<CurrentMemoryManagerStategy> CCurrentManagerAllocOn;
