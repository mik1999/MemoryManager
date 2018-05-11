#pragma once

#include <memory>
#include <iostream>
#include <stack>
#include <cassert>
#include <cstddef>

#include "IMemoryManager.h"
#include "Stack.h"
#include "CAllocatedOn.h"

struct alignas(alignof(std::max_align_t)) AllocationSituation : public CRuntimeHeapAllocOn {
	enum Condition {
		UsingManager,
		Default,
		Invalid
	};
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
	Stack<AllocationSituation*> _createdSituations;
};

namespace CMemoryManagerUtilityNamespace {};