#pragma once

#include <memory>

class RuntimeHeapStategy {
public:
	static void* alloc(size_t size);
	static void free(void* ptr);
};

class CurrentMemoryManagerStategy {
public:
	static void* alloc(size_t size);
	static void free(void* ptr);
};