#include <iostream>

#include <memory>
#include <iostream>

#include <vector>
#include <map>
#include <stack>

using namespace std;


#include "IMemoryManager.h"
#include "CMemoryManagerSwitcher.h"
#include "AllocatorBasedManager.h"
#include "../../XorList/XorList/StackAllocator.h"


AllocatorBasedManager <std::allocator<char> > STDallocManager;

void doSomething() {
	AllocatorBasedManager <std::allocator<char> > STDallocManager;
	CMemoryManagerSwitcher switcher;
	//vector <int> v;
	stack <int> st;
	switcher.switchTo(STDallocManager);
	int *i = new int;
	delete i;
	switcher.switchToDefault();
}

int main() {
	doSomething();
	system("pause");
	return 0;
}