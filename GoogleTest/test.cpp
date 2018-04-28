#include "pch.h"

#include <algorithm>
#include <random>
#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <iostream>
#include <stack>
#include <assert.h>
#include <map>

#include "../MemoryManager/Stack.h"

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
	}
private:
	using value_type = typename Allocator::value_type;
	using charAllocator = typename Allocator::template rebind<char>::other;
	using mapAllocator = typename Allocator::template rebind<std::pair<const void*, size_t> >::other;
	charAllocator _allocator;
	std::map<void*, size_t, std::less<void*>, mapAllocator> _allocSize;
};

#define DEBUG
class IMemoryManager {
public:
	void* alloc(size_t size);
	void free(void* ptr);
	IMemoryManager();
	virtual ~IMemoryManager();
#ifdef DEBUG
	void  setDebugBreakPoint(size_t allocNum);
#endif // DEBUG
private:
	virtual void* internalAlloc(size_t size) = 0;
	virtual void internalFree(void* ptr) = 0;
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


std::random_device rd;

void testSimpleAllocations() {
	int* ptr = new int(1);
	int* array = new int[10];
	*ptr = 5;
	array[5] = 7;
	delete ptr;
	delete[] array;
}

template <typename T>
void testAllocations() {
	const int SIZE = 15;
	T* ptr = new T;
	*ptr = T();
	T other = *ptr;
	T* array = new T[SIZE];
	for (int i = 0; i < 10; i++)
	array[rd() % SIZE] = T();
	delete ptr;
	delete[] array;
}

void testComplicatedTypes() {
	testAllocations<int>();
	testAllocations<long double>();
	testAllocations<unsigned int>();
	testAllocations<std::vector <char> >();
	testAllocations<std::set <size_t> >();
	testAllocations<std::queue<std::pair<std::vector<int>, std::allocator<double> > > >();
}

void testBigMemoryAllocations() {
	const int SIZE = 1000000;
	for (int i = 0; i < 700; i++) {
		double* array = new double[SIZE];
		for (int j = 0; j < 30; j++) {
			size_t index = rd() % SIZE;
			array[index] = rd();
		}
	delete[] array;
	}

}

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
	Stack<AllocationSituation*> _createdSituations;
};



namespace CMemoryManagerUtilityNamespace {

	Stack<AllocationSituation> situations;

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

TEST(DefaultAllocationTestCase, SimpleTest) {
	testSimpleAllocations();
}

TEST(DefaultAllocationTestCase, ComplicatedDataTest) {
	testComplicatedTypes();
}

TEST(DefaultAllocationTestCase, DISABLED_BigMemoryTest) {
	testBigMemoryAllocations();
}


TEST(STDallocSwitchedCase, DISABLED_MainTest) {
	AllocatorBasedManager <std::allocator<char> > STDallocManager;
	CMemoryManagerSwitcher switcher;
	switcher.switchTo(STDallocManager);
	testSimpleAllocations();
	testComplicatedTypes();
	testBigMemoryAllocations();
}

TEST(defaultAllocationAgainCase, Main) {
	CMemoryManagerSwitcher switcher;
	AllocatorBasedManager<std::allocator<char> > STDallocManager;
	switcher.switchTo(STDallocManager);
	testComplicatedTypes();
	switcher.switchToDefault();
	testSimpleAllocations();
	testComplicatedTypes();
	testBigMemoryAllocations();
}


int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
