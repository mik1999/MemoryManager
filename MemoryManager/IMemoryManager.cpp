#include "IMemoryManager.h"

void * IMemoryManager::alloc(size_t size)
{
#ifdef DEBUG
	_currentAllocNum++;
	if (_breakPoints.count(_currentAllocNum) == 1)
		throw std::bad_alloc();
	void* ans = internalAlloc(size);
	_allocations[ans] = Allocation(_currentAllocNum, size, ans);
	return ans;

#else
	return internalAlloc(size);
#endif // DEBUG
}

void IMemoryManager::free(void * ptr)
{
#ifdef DEBUG
	_allocations.erase(ptr);
	internalFree(ptr);
#else
	internalFree(ptr);
#endif // DEBUG
}

IMemoryManager::IMemoryManager()
{
#ifdef DEBUG
	_currentAllocNum = 0;
#else
	//do nothing
#endif // DEBUG
}

IMemoryManager::~IMemoryManager()
{
#ifdef DEBUG
	for (auto it = _allocations.begin(); it != _allocations.end(); it++)
		std::cerr << "Memory sector haven't been freed! Allocation number: "
		<< (*it).second.number << ", size: "
		<< (*it).second.size << ", ptr: " << (*it).second.ptr << std::endl;
#else
	//do nothing
#endif // DEBUG
}

#ifdef DEBUG
void IMemoryManager::setDebugBreakPoint(size_t allocNum)
{
	_breakPoints.insert(allocNum);
}
#endif // DEBUG