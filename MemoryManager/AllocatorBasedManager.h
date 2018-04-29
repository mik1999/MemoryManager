#pragma once
#include "IMemoryManager.h"
#include <memory>
#include <map>
#include <iostream>

template <class Allocator>
class AllocatorBasedManager : public IMemoryManager {
public:
	AllocatorBasedManager() {}
	virtual ~AllocatorBasedManager() {}
	virtual void* internalAlloc(size_t size) {
		void* ans = reinterpret_cast<void*> (_allocator.allocate(size));
		_allocSize[ans] = size;
		return ans;
	}

	virtual void internalFree(void* ptr) {
		_allocator.deallocate(reinterpret_cast<char*>(ptr), _allocSize[ptr]);
		_allocSize.erase(ptr);
	}
private:
	using value_type = typename Allocator::value_type;
	using charAllocator = typename Allocator::template rebind<char>::other;
	using mapAllocator = typename Allocator::template rebind<std::pair<const void*, size_t> >::other;
	charAllocator _allocator;
	std::map<void*, size_t, std::less<void*>, mapAllocator> _allocSize;
};