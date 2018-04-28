#include <utility>
#include <iostream>
#include <fstream>

template <typename T>
class Stack;

template <typename T>
std::ostream& operator << (std::ostream& out, const Stack<T>& stack);

template <typename T>
bool operator ==(const Stack<T>& fst, const Stack<T>& scd);

template <typename T>
class Stack {
public:
	Stack();
	~Stack();
	Stack(const Stack & other);
	Stack& operator = (const Stack & other);
	Stack& operator = (Stack && other);
	Stack(Stack && other);

	void *operator new(size_t size);
	void operator delete(void *ptr);
	void *operator new[](size_t size);
	void operator delete[](void *ptr);

	T & top();
	const T & top() const;
	void pop();
	void push(const T & value);
	void push(T && value);
	bool empty();
private:
	struct Node {
		Node* prev;
		T key;
		Node() {}
		Node(Node *prev, const T & key) : prev(prev), key(key) {}
		void *operator new(size_t size)
		{
			return malloc(size);
		}

		void operator delete(void *ptr)
		{
			free(ptr);
		}

		void *operator new[](size_t size)
		{
			return malloc(size);
		}
			
		void operator delete[](void *ptr)
		{
			free(ptr);
		}
	};
	Node* _back;
	void shiftElements(Stack & other);
	void copyElements(const Stack & other);
	friend bool operator == <T>(const Stack& fst, const Stack& scd);
	friend std::ostream& operator << <T>(std::ostream& out, const Stack<T>& stack);
};

template<typename T>
inline Stack<T>::Stack()
{
	_back = nullptr;
}

template<typename T>
inline Stack<T>::~Stack()
{
	while (!empty())
		pop();
}

template<typename T>
inline Stack<T>::Stack(const Stack & other)
{
	copyElements(other);
}

template<typename T>
inline Stack<T> & Stack<T>::operator=(const Stack & other)
{
	copyElements(other);
	return *this;
}

template<typename T>
inline Stack<T> & Stack<T>::operator=(Stack && other)
{
	_back = other._back;
	other._back = nullptr;
}

template<typename T>
inline Stack<T>::Stack(Stack && other) : _back(other._back)
{
	//initialize values
}

template<typename T>
inline void * Stack<T>::operator new(size_t size)
{
	return malloc(size);
}

template<typename T>
inline void Stack<T>::operator delete(void * ptr)
{
	free(ptr);
}

template<typename T>
inline void * Stack<T>::operator new[](size_t size)
{
	return malloc(size);
}

template<typename T>
inline void Stack<T>::operator delete[](void * ptr)
{
	free(ptr);
}

template<typename T>
T & Stack<T>::top()
{
	return _back->key;
}

template<typename T>
const T & Stack<T>::top() const
{
	return _back->key;
}

template<typename T>
void Stack<T>::pop()
{
	Node *oldBack = _back;
	_back = _back->prev;
	delete oldBack;
}

template<typename T>
inline void Stack<T>::push(const T & value)
{
	Node *newNode = new Node(_back, value);
	_back = newNode;
}

template<typename T>
inline void Stack<T>::push(T && value)
{
	Node *newNode = new Node;
	newNode->prev = _back;
	newNode->key = std::move<T>(std::forward<T>(value));
	_back = newNode;
}

template<typename T>
bool Stack<T>::empty()
{
	return _back == nullptr;
}

template<typename T>
inline void Stack<T>::shiftElements(Stack & other)
{
	Node* curNode = other._back;
	while (curNode != nullptr) {
		Node* prev = curNode->prev;
		curNode->prev = _back;
		_back = curNode;
		curNode = prev;
	}
	other._back = nullptr;
}

template<typename T>
inline void Stack<T>::copyElements(const Stack & other)
{
	Stack intermediate;
	Node* node = other._back;
	while (node != nullptr) {
		intermediate.push(node->key);
		node = node->prev;
	}
	shiftElements(intermediate);
}

template <typename T>
std::ostream& operator << (std::ostream& out, const Stack<T>& stack) {
	Stack<T> intermediate;
	Stack<T>::Node *node = stack._back;
	while (node != nullptr) {
		intermediate.push(node->key);
		node = node->prev;
	}
	while (!intermediate.empty()) {
		out << intermediate.top() << ' ';
		intermediate.pop();
	}
	out << '\n';
	return out;
}

template <typename T>
bool operator ==(const Stack<T>& fst, const Stack<T>& scd) {
	Stack<T>::Node *fstNode = fst._back, *scdNode = scd._back;
	while (fstNode != nullptr && scdNode != nullptr) {
		if (fstNode->key != scdNode->key)
			return false;
		fstNode = fstNode->prev;
		scdNode = scdNode->prev;
	}
	if (fstNode != nullptr || scdNode != nullptr)
		return false;
	return true;
}

template <typename T>
bool operator !=(const Stack<T>& fst, const Stack<T>& scd) {
	return !(fst == scd);
}