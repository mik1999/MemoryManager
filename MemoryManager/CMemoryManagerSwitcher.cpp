#include "CMemoryManagerSwitcher.h"

namespace CMemoryManagerUtilityNamespace {

	std::stack<AllocationSituation> situations;

	bool forciblySwitchedToDefault = false;

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
		return forciblySwitchedToDefault || situations.empty() ||
			current().condition == AllocationSituation::Default;
	}

	const size_t EXTRA_MEMORY_SIZE = sizeof(AllocationSituation);
}

using namespace CMemoryManagerUtilityNamespace;

void * AllocationSituation::operator new(size_t size)
{
	return malloc(size);
}

void AllocationSituation::operator delete(void * ptr)
{
	free(ptr);
}

void * AllocationSituation::operator new[](size_t size)
{
	return malloc(size);
}

void AllocationSituation::operator delete[](void * ptr)
{
	free(ptr);
}

void * AllocationSituation::operator new(size_t size, void * ptr)
{
	return ptr;
}

void AllocationSituation::operator delete(void * ptr, void * place)
{
	//do nothing
}

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
			forciblySwitchedToDefault = true; //alloc may use ::new()
			ptr = current().manager->alloc(size + EXTRA_MEMORY_SIZE);
			forciblySwitchedToDefault = false;
			new (reinterpret_cast<AllocationSituation*>(ptr)) AllocationSituation(
				current()
			);
		}
		return reinterpret_cast<void*>(reinterpret_cast<char*>(ptr) + EXTRA_MEMORY_SIZE);
	}

	inline void myFree(void* ptr) {
		char* blockBegin = reinterpret_cast<char*>(ptr) - EXTRA_MEMORY_SIZE;
		AllocationSituation* situation =
			reinterpret_cast<AllocationSituation*>(blockBegin);
		if (situation->condition == AllocationSituation::Default) {
			free(reinterpret_cast<void*>(blockBegin));
		}
		else {
			IMemoryManager* manager = situation->manager;
			forciblySwitchedToDefault = true; //free may use ::new and ::delete
			manager->free(reinterpret_cast<void*>(blockBegin));
			forciblySwitchedToDefault = false;
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

void *operator new[](size_t size)
{
	return myMalloc(size);
}

void operator delete[](void *ptr)
{
	myFree(ptr);
}
