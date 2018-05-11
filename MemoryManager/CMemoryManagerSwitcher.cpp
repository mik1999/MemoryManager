#include "CMemoryManagerSwitcher.h"

namespace CMemoryManagerUtilityNamespace {

	Stack<AllocationSituation> situations;

	void renew() {
		while (!situations.empty() &&
			situations.top().condition == AllocationSituation::Invalid)
			situations.pop();
	}

	AllocationSituation& current() {
		renew();
		return situations.top();
	}

	bool isDefault() {
		renew();
		return IMemoryManager::isSwitchedToDefault() || situations.empty() ||
			current().condition == AllocationSituation::Default;
	}

	const size_t EXTRA_MEMORY_SIZE = sizeof(AllocationSituation);
}

using namespace CMemoryManagerUtilityNamespace;

AllocationSituation::AllocationSituation(Condition cond, IMemoryManager * ptrManager) :
	condition(cond), manager(ptrManager)
{
	//initialize values
}

CMemoryManagerSwitcher::~CMemoryManagerSwitcher()
{
	while (!_createdSituations.empty()) {
		_createdSituations.top()->condition = AllocationSituation::Invalid;
		_createdSituations.pop();
	}
}

void * CMemoryManagerSwitcher::operator new(size_t size)
{
	return malloc(size);
}

void CMemoryManagerSwitcher::operator delete(void * ptr)
{
	free(ptr);
}

void CMemoryManagerSwitcher::switchTo(IMemoryManager & manager)
{
	CMemoryManagerUtilityNamespace::
		situations.push(AllocationSituation(
			AllocationSituation::UsingManager, &manager
		));
	_createdSituations.push(&current());
}

void CMemoryManagerSwitcher::switchToDefault()
{
	CMemoryManagerUtilityNamespace::
		situations.push(AllocationSituation(
			AllocationSituation::Default, nullptr
		));
	_createdSituations.push(&current());
}


namespace CMemoryManagerUtilityNamespace {

	inline void *myMalloc(size_t size) {
		void* ptr;
		if (isDefault()) {
			ptr = malloc(size + EXTRA_MEMORY_SIZE);
			new (reinterpret_cast<AllocationSituation*>(ptr)) AllocationSituation(
				AllocationSituation::Default, nullptr
			);
		}
		else {
			ptr = current().manager->alloc(size + EXTRA_MEMORY_SIZE);
			new (reinterpret_cast<AllocationSituation*>(ptr)) AllocationSituation(
				current()
			);
		}
		return reinterpret_cast<void*>(reinterpret_cast<char*>(ptr) + EXTRA_MEMORY_SIZE);
	}

	inline void myFree(void* ptr) {
		char* blockBegin = reinterpret_cast<char*>(ptr) - EXTRA_MEMORY_SIZE;
		auto situation =
			reinterpret_cast<AllocationSituation*>(blockBegin);
		if (situation->condition == AllocationSituation::Default) {
			free(reinterpret_cast<void*>(blockBegin));
		}
		else {
			IMemoryManager* manager = situation->manager;
			manager->free(reinterpret_cast<void*>(blockBegin));
		}
	}
}

void *operator new(size_t size)
{
	return myMalloc(size);
}

void operator delete(void *ptr)
{
	myFree(ptr);
}

void operator delete(void *ptr, size_t size)
{
	myFree(ptr);
}


void *operator new[](size_t size)
{
	return myMalloc(size);
}

void operator delete[](void *ptr)
{
	myFree(ptr);
}
