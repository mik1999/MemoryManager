#include "AllocationStrategy.h"

void * RuntimeHeapStategy::alloc(size_t size)
{
	return ::malloc(size);
}

void RuntimeHeapStategy::free(void * ptr)
{
	::free(ptr);
}

void * CurrentMemoryManagerStategy::alloc(size_t size)
{
	return reinterpret_cast<void*>(new char[size]);
}

void CurrentMemoryManagerStategy::free(void * ptr)
{
	delete[] ptr;
}
