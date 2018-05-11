#pragma once

#include <map>
#include <set>
#include <iostream>

#define DEBUG
class IMemoryManager {
public:
	void* alloc(size_t size);
	void free(void* ptr);
	IMemoryManager();
	virtual ~IMemoryManager();
#ifdef DEBUG
	void  setDebugBreakPoint(size_t allocNum);
	static bool isSwitchedToDefault();
#endif // DEBUG
private:
	virtual void* internalAlloc(size_t size) = 0;
	virtual void internalFree(void* ptr) = 0;
	static bool forciblySwitchedToDefault;
#ifdef DEBUG
	size_t _currentAllocNum;
	struct Allocation {
		Allocation() {}
		Allocation(size_t number, size_t size, void* ptr) : number(number), size(size), ptr(ptr) {}
		size_t number;
		size_t size;
		void* ptr;
	};
	std::map<void*, Allocation> _allocations;
	std::set<size_t> _breakPoints;
#endif // DEBUG
};