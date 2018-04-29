#include "pch.h"
#include <random>
#include <vector>
#include <queue>

#include "../MemoryManager/CMemoryManagerSwitcher.h"
#include "../MemoryManager/AllocatorBasedManager.h"
#include "../MemoryManager/CAllocatedOn.h"

#include "../../XorList/XorList/StackAllocator.h"
#include "../../XorList/XorList/ListOperation.h"

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

TEST(DefaultAllocationTestCase, SimpleTest) {
	testSimpleAllocations();
}

TEST(DefaultAllocationTestCase, ComplicatedDataTest) {
	testComplicatedTypes();
}

TEST(DefaultAllocationTestCase, DISABLED_BigMemoryTest) {
	testBigMemoryAllocations();
}


TEST(SwitchToAllocatorCase, DISABLED_Simple) {
	AllocatorBasedManager <std::allocator<char> > STDallocManager;
	CMemoryManagerSwitcher switcher;
	std::stack<std::vector<int> > st;
	st.push(std::vector<int>(7));
	st.push(std::vector<int>(17));
	switcher.switchTo(STDallocManager);
	st.pop();
	st.push(std::vector<int>(13));
	testSimpleAllocations();
	testComplicatedTypes();
	testBigMemoryAllocations();
}

TEST(SwitchToAllocatorCase, DISABLED_DefaultAllocAgain) {
	CMemoryManagerSwitcher switcher;
	AllocatorBasedManager<std::allocator<char> > STDallocManager;
	switcher.switchTo(STDallocManager);
	testComplicatedTypes();
	switcher.switchToDefault();
	testSimpleAllocations();
	testComplicatedTypes();
	testBigMemoryAllocations();
}

TEST(SwitchToAllocatorCase, FewAllocatorsTest) {
	CMemoryManagerSwitcher switcher;

	testComplicatedTypes();

	{
		AllocatorBasedManager<std::allocator<char> > charManager;
		switcher.switchTo(charManager);
		testComplicatedTypes();
	}

	switcher.switchToDefault();
	testComplicatedTypes();

	{
		AllocatorBasedManager<std::allocator<int> > intManager;
		switcher.switchTo(intManager);
		testComplicatedTypes();
	}

	switcher.switchToDefault();
	testComplicatedTypes();


	{
		AllocatorBasedManager<StackAllocator<double> > stackManager;
		switcher.switchTo(stackManager);
		testComplicatedTypes();
	}

	switcher.switchToDefault();
	testComplicatedTypes();

}

TEST(SwitcherRAII, Test1) {
	{
		CMemoryManagerSwitcher switcher;
		AllocatorBasedManager<std::allocator<char> > STDallocManager;
		switcher.switchTo(STDallocManager);
		testComplicatedTypes();
	}
	testComplicatedTypes();
	{		
		CMemoryManagerSwitcher switcher;
		AllocatorBasedManager<std::allocator<char> > manager1;
		AllocatorBasedManager<StackAllocator<double> > manager2;
		std::map<int, int> m;
		m[5] = 7;
		switcher.switchTo(manager1);
		m[4] = 8;
		switcher.switchTo(manager2);
		m[10] = -1;
		switcher.switchTo(manager1);
		m[-1] = 0;
	}
	testSimpleAllocations();
	{
		CMemoryManagerSwitcher switcher1;
		AllocatorBasedManager<std::allocator<char> > manager1;
		switcher1.switchTo(manager1);
		int* ptr = new int; //ptr must be deleted with manager1
		{
			CMemoryManagerSwitcher switcher2;
			AllocatorBasedManager<StackAllocator<int> > manager2;
			switcher2.switchTo(manager2);
			testComplicatedTypes();
			switcher2.switchToDefault();
		}
		delete ptr;
	}
}

double workingTime(std::list<ListOperation<int> > &ops) {
	clock_t begTime = clock();
	std::list<int> l;
	for (auto op : ops)
		op(l);
	clock_t endTime = clock();
	return double(endTime - begTime) / CLOCKS_PER_SEC;
}

TEST(WorkingTimeCompare, DISABLED_Main) {
	CMemoryManagerSwitcher switcher;
	AllocatorBasedManager<std::allocator<char> > manager1;
	AllocatorBasedManager<std::allocator<long long> > manager2;
	AllocatorBasedManager<StackAllocator<char> > manager3;
	auto ops = generateRandomLeapOperations<int, rand>(1000000);
	std::ofstream workingTimeCompareResult("workingTimeCompareResult.txt");
	switcher.switchToDefault();
	workingTimeCompareResult << std::fixed << std::setprecision(2);
	workingTimeCompareResult << "Default: " << workingTime(ops) << "\n";
	switcher.switchTo(manager1);
	workingTimeCompareResult << "STD allocator on char: " << workingTime(ops) << "\n";
	switcher.switchTo(manager2);
	workingTimeCompareResult << "STD allocator on long long: " << workingTime(ops) << "\n";
	switcher.switchTo(manager3);
	workingTimeCompareResult << "StackAllocator: " << workingTime(ops) << "\n";
}

class NarrowMemoryManager : public IMemoryManager {
public:
	void* internalAlloc(size_t size) {
		if (size + totalSize > MAX_SIZE)
			throw std::bad_alloc();
		size += totalSize;
		return malloc(size);
	}
	void internalFree(void* ptr) {
		free(ptr);
	}
private:
	size_t totalSize;
	static const size_t MAX_SIZE = 100;
};

class SimpleAllocOnRuntimeHeap : public CRuntimeHeapAllocOn {
public:
	SimpleAllocOnRuntimeHeap() {
		someData1 = 7;
		someData2 = 42;
	}
	void someWork() {
		someData1 += someData2;
		someData2 = someData2 - someData1;
		someData1 -= someData2;
	}
private:
	int someData1;
	int someData2;
};

class SimpleAllocOnDefaultManager : public CCurrentManagerAllocOn {
private:
	int someData;
};

template <class C>
void AllocateSomeData(){
	C* data = new C[200];
	data[90] = C();
	new(data + 70) C();
	delete[] data;
}

TEST(CAllocatedOn, Test1) {
	CMemoryManagerSwitcher switcher;
	NarrowMemoryManager manager;
	switcher.switchTo(manager);
	// assert that new SimpleAllocOnDefaultManager doesn't call ::new 
	ASSERT_THROW(AllocateSomeData<SimpleAllocOnDefaultManager>(), std::bad_alloc);
}

TEST(CAllocatedOn, Test2) {
	CMemoryManagerSwitcher switcher;
	NarrowMemoryManager manager;
	switcher.switchTo(manager);
	AllocateSomeData<SimpleAllocOnRuntimeHeap>();
	ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
