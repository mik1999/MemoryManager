#pragma once

#include <memory>
#include <iostream>
#include <stack>
#include <assert.h>

#include "IMemoryManager.h"
struct alignas(alignof(std::max_align_t)) AllocationSituation {
	enum Condition {
		UsingManager,
		Default,
		Invalid
	};
	void *operator new(size_t size);
	void operator delete(void *ptr);
	void *operator new[](size_t size);
	void operator delete[](void *ptr);
	void *operator new(size_t size, void *p);
	void operator delete  (void* ptr, void* place);
	AllocationSituation() = default;
	AllocationSituation(Condition cond, IMemoryManager* ptrManager);
	Condition condition;
	IMemoryManager* manager;
};

class CMemoryManagerSwitcher {
public:
	CMemoryManagerSwitcher() = default;
	CMemoryManagerSwitcher(const CMemoryManagerSwitcher& other) = delete;
	~CMemoryManagerSwitcher();

	CMemoryManagerSwitcher& operator =(const CMemoryManagerSwitcher& other) = delete;

	static void *operator new(size_t size);

	static void operator delete(void *ptr);

	void switchTo(IMemoryManager &manager);
	void switchToDefault();
private:
	std::stack<AllocationSituation*> _createdSituations;
};
