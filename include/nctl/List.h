#ifndef CLASS_NCTL_LIST
#define CLASS_NCTL_LIST

#include "algorithms.h"
#include "ListIterator.h"
#include "ReverseIterator.h"

namespace nctl {

template <class T> class List;

/// A list node based on templates
template <class T>
class ListNode
{
  public:
	/// Data payload for the node
	T data_;
	/// A pointer to the previous node in the list
	ListNode *previous_;
	/// A pointer to the next node in the list
	ListNode *next_;

  private:
	ListNode(const T &data, ListNode *previous, ListNode *next)
		: data_(data), previous_(previous), next_(next) { }

	friend class List<T>;
};

/// A double linked list based on templates
template <class T>
class List
{
  public:
	/// Iterator type
	using Iterator = ListIterator<T, false>;
	/// Constant iterator type
	using ConstIterator = ListIterator<T, true>;
	/// Reverse iterator type
	using ReverseIterator = nctl::ReverseIterator<Iterator>;
	/// Reverse constant iterator type
	using ConstReverseIterator = nctl::ReverseIterator<ConstIterator>;

	List() : size_(0), head_(nullptr), tail_(nullptr) { }
	~List() { clear(); }

	/// Copy constructor
	List(const List &other);
	/// Copy-and-swap assignment operator
	List &operator=(List other);

	/// Swaps two lists without copying their data
	void swap(List &first, List &second)
	{
		nctl::swap(first.size_, second.size_);
		nctl::swap(first.head_, second.head_);
		nctl::swap(first.tail_, second.tail_);
	}

	/// Returns an iterator to the first element
	inline Iterator begin() { return Iterator(head_); }
	/// Returns a reverse iterator to the last element
	inline ReverseIterator rBegin() { return ReverseIterator(Iterator(tail_)); }
	/// Returns an iterator to the end of the list sentinel (valid for reverse traversal too)
	inline Iterator end() { return Iterator(nullptr); }
	/// Returns a reverse iterator to the end of the list sentinel
	/*! It exists only for coherency with the rest of the containers. */
	inline ReverseIterator rEnd() { return ReverseIterator(end()); }

	/// Returns a constant iterator to the first element
	inline ConstIterator begin() const { return ConstIterator(head_); }
	/// Returns a constant reverse iterator to the last element
	inline ConstReverseIterator rBegin() const { return ConstReverseIterator(ConstIterator(tail_)); }
	/// Returns a constant iterator to the end of the list sentinel (valid reverse traversal too)
	inline ConstIterator end() const { return ConstIterator(nullptr); }
	/// Returns a constant reverse iterator to the end of the list sentinel
	/*! It exists only for coherency with the rest of the containers. */
	inline ConstReverseIterator rEnd() const { return ConstReverseIterator(end()); }

	/// Returns true if the list is empty
	inline bool isEmpty() const { return head_ == nullptr; }
	/// Returns the number of elements in the list
	inline unsigned int size() const { return size_; }
	/// Clears the list
	void clear();
	/// Returns a constant reference to the first element in constant time
	inline const T &front() const { ASSERT(head_); return head_->data_; }
	/// Returns a reference to the first element in constant time
	inline T &front() { ASSERT(head_); return head_->data_; }
	/// Returns a constant reference to the last element in constant time
	inline const T &back() const { ASSERT(tail_); return tail_->data_; }
	/// Returns a reference to the last element in constant time
	inline T &back() { ASSERT(tail_); return tail_->data_; }
	/// Inserts a new element as the first, in constant time
	void pushFront(const T &element);
	/// Inserts a new element as the last, in constant time
	void pushBack(const T &element);
	/// Removes the first element in constant time
	void popFront() { removeNode(head_); }
	/// Removes the last element in constant time
	void popBack() { removeNode(tail_); }
	/// Inserts a new element after the node pointed by the constant iterator
	ConstIterator insertAfter(const Iterator position, const T &element);
	/// Inserts a new element before the node pointed by the constant iterator
	ConstIterator insertBefore(const Iterator position, const T &element);
	/// Inserts new elements from a source range after the node pointed by the constant iterator, last not included
	ConstIterator insert(const Iterator position, Iterator first, const Iterator last);
	/// Removes the node pointed by the constant iterator in constant time
	ConstIterator erase(ConstIterator position);
	/// Removes the range of nodes pointed by the iterators in constant time
	ConstIterator erase(ConstIterator first, const ConstIterator last);
	/// Removes a specified element in linear time
	void remove(const T &element);
	/// Removes all the elements that fulfill the condition
	template <class Predicate> void removeIf(Predicate pred);
	/// Transfers all the elements from the source list in front of `position`
	void splice(Iterator position, List &source);
	/// Transfers one element at `it` from the source list in front of `position`
	void splice(Iterator position, List &source, Iterator it);
	/// Transfers a range of elements from the source list, `last` not included, in front of `position`
	void splice(Iterator position, List &source, Iterator first, Iterator last);

  private:
	/// Number of elements in the list
	unsigned int size_;
	/// Pointer to the first node in the list
	ListNode<T> *head_;
	/// Pointer to the last node in the list
	ListNode<T> *tail_;

	/// Inserts a new element after a specified node
	ListNode<T> *insertAfterNode(ListNode<T> *node, const T &element);
	/// Inserts a new element before a specified node
	ListNode<T> *insertBeforeNode(ListNode<T> *node, const T &element);
	/// Removes a specified node in constant time
	ListNode<T> *removeNode(ListNode<T> *node);
	/// Removes a range of nodes in constant time, last not included
	ListNode<T> *removeRange(ListNode<T> *firstNode, ListNode<T> *lastNode);
};

template <class T>
List<T>::List(const List<T> &other)
	: size_(0), head_(nullptr), tail_(nullptr)
{
	for (List<T>::ConstIterator i = other.begin(); i != other.end(); ++i)
		pushBack(*i);
}

/*! The parameter should be passed by value for the idiom to work. */
template <class T>
List<T> &List<T>::operator=(List<T> other)
{
	swap(*this, other);
	return *this;
}

template <class T>
void List<T>::clear()
{
	ListNode<T> *next = nullptr;

	while (head_)
	{
		next = head_->next_;
		delete head_;
		head_ = next;
	}

	tail_ = nullptr;
	size_ = 0;
}

template <class T>
void List<T>::pushFront(const T &element)
{
	ListNode<T> *node = new ListNode<T>(element, nullptr, head_);
	if (head_)
		head_->previous_ = node;
	head_ = node;

	// The list is empty
	if (tail_ == nullptr)
		tail_ = node;
	size_++;
}

template <class T>
void List<T>::pushBack(const T &element)
{
	ListNode<T> *node = new ListNode<T>(element, tail_, nullptr);
	if (tail_)
		tail_->next_ = node;
	tail_ = node;

	// The list is empty
	if (head_ == nullptr)
		head_ = node;
	size_++;
}

template <class T>
inline typename List<T>::ConstIterator List<T>::insertAfter(const Iterator position, const T &element)
{
	return ConstIterator(insertAfterNode(position.node_, element));
}

template <class T>
inline typename List<T>::ConstIterator List<T>::insertBefore(const Iterator position, const T &element)
{
	return ConstIterator(insertBeforeNode(position.node_, element));
}

template <class T>
typename List<T>::ConstIterator List<T>::insert(Iterator position, Iterator first, const Iterator last)
{
	ListNode<T> *node = position.node_;
	while (first != last)
	{
		node = insertAfterNode(node, *first);
		++first;
	}

	return ConstIterator(node);
}

/*! \note The iterator cannot be used after on. */
template <class T>
inline typename List<T>::ConstIterator List<T>::erase(ConstIterator position)
{
	ListNode<T> *nextNode = removeNode(position.node_);
	return ConstIterator(nextNode);
}

/*! \note The first iterator cannot be used after on. */
template <class T>
inline typename List<T>::ConstIterator List<T>::erase(ConstIterator first, const ConstIterator last)
{
	ListNode<T> *nextNode = removeRange(first.node_, last.node_);
	return ConstIterator(nextNode);
}

template <class T>
void List<T>::remove(const T &element)
{
	ListNode<T> *current = head_;

	while (current)
	{
		if (current->data_ == element)
		{
			removeNode(current);
			break;
		}

		current = current->next_;
	}
}

template <class T> template <class Predicate>
void List<T>::removeIf(Predicate pred)
{
	ConstIterator i = begin();
	while (i != end())
	{
		if (pred(*i))
			i = erase(i);
		else
			++i;
	}
}

template <class T>
void List<T>::splice(Iterator position, List &source)
{
	ListNode<T> *node = position.node_;

	// Cannot attach at the back because the end iterator is
	// only a sentinel, cannot be decremented by one
	if (node == nullptr && size_ > 0)
		return;

	if (node)
	{
		ListNode<T> *prevNode = node->previous_;
		node->previous_ = source.tail_;
		source.tail_->next_ = node;
		source.head_->previous_ = prevNode;
		if (prevNode)
			prevNode->next_ = source.head_;
		else
			head_ = source.head_;
	}
	else
	{
		// The destination list is empty
		head_ = source.head_;
		tail_ = source.tail_;
	}
	size_ += source.size_;

	source.head_ = nullptr;
	source.tail_ = nullptr;
	source.size_ = 0;
}

template <class T>
void List<T>::splice(Iterator position, List &source, Iterator it)
{
	ListNode<T> *node = position.node_;
	ListNode<T> *sourceNode = it.node_;

	// Cannot attach at the back because the end iterator is
	// only a sentinel, cannot be decremented by one
	if (node == nullptr && size_ > 0)
		return;

	// Early-out if there is nothing to transfer
	if (sourceNode == nullptr)
		return;

	ListNode<T> *sourcePrev = sourceNode->previous_;
	ListNode<T> *sourceNext = sourceNode->next_;
	if (sourcePrev == nullptr)
		source.head_ = sourceNode->next_;
	else
		sourcePrev->next_ = sourceNext;

	if (sourceNode->next_ == nullptr)
		source.tail_ = sourceNode->previous_;
	else
		sourceNext->previous_ = sourcePrev;

	if (node)
	{
		ListNode<T> *prevNode = node->previous_;
		node->previous_ = sourceNode;
		sourceNode->previous_ = prevNode;
		sourceNode->next_ = node;
		if (prevNode)
			prevNode->next_ = sourceNode;
		else
			head_ = sourceNode;
	}
	else
	{
		head_ = sourceNode;
		tail_ = sourceNode;
		sourceNode->previous_ = nullptr;
		sourceNode->next_ = nullptr;
	}
	size_++;
	source.size_--;
}

template <class T>
void List<T>::splice(Iterator position, List &source, Iterator first, Iterator last)
{
	ListNode<T> *node = position.node_;
	ListNode<T> *firstNode = first.node_;

	// Cannot attach at the back because the end iterator is
	// only a sentinel, cannot be decremented by one
	if (node == nullptr && size_ > 0)
		return;

	// Early-out if there is nothing to transfer
	if (firstNode == nullptr)
		return;

	ListNode<T> *firstPrev = firstNode->previous_;
	ListNode<T> *lastIncludedNode = first.node_;
	while (first != last)
	{
		lastIncludedNode = first.node_;

		++first;
		size_++;
		source.size_--;
	}

	if (firstPrev == nullptr)
		source.head_ = lastIncludedNode->next_;
	else
		firstPrev->next_ = lastIncludedNode->next_;

	if (lastIncludedNode->next_ == nullptr)
		source.tail_ = firstPrev;
	else
		lastIncludedNode->next_->previous_ = firstPrev;

	if (node)
	{
		ListNode<T> *prevNode = node->previous_;
		node->previous_ = lastIncludedNode;
		firstNode->previous_ = prevNode;
		lastIncludedNode->next_ = node;
		if (prevNode)
			prevNode->next_ = firstNode;
		else
			head_ = firstNode;
		lastIncludedNode->next_ = node;
	}
	else
	{
		head_ = firstNode;
		tail_ = lastIncludedNode;
		firstNode->previous_ = nullptr;
		lastIncludedNode->next_ = nullptr;
	}
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

template <class T>
ListNode<T> *List<T>::insertAfterNode(ListNode<T> *node, const T &element)
{
	if (head_ == nullptr && node == nullptr)
	{
		pushBack(element);
		return head_;
	}

	ListNode<T> *newNode = new ListNode<T>(element, node, node->next_);

	if (node->next_ == nullptr)
		tail_ = newNode;
	else
		node->next_->previous_ = newNode;

	node->next_ = newNode;
	size_++;

	return newNode;
}

template <class T>
ListNode<T> *List<T>::insertBeforeNode(ListNode<T> *node, const T &element)
{
	if (tail_ == nullptr && node == nullptr)
	{
		pushFront(element);
		return tail_;
	}

	ListNode<T> *newNode = new ListNode<T>(element, node->previous_, node);

	if (node->previous_ == nullptr)
		head_ = newNode;
	else
		node->previous_->next_ = newNode;

	node->previous_ = newNode;
	size_++;

	return newNode;
}

template <class T>
ListNode<T> *List<T>::removeNode(ListNode<T> *node)
{
	// Early-out to prevent `nullptr` dereferencing
	if (node == nullptr)
		return nullptr;

	if (node->previous_)
		node->previous_->next_ = node->next_;
	else // removing the head
		head_ = node->next_;

	if (node->next_)
		node->next_->previous_ = node->previous_;
	else // removing the tail
		tail_ = node->previous_;
	size_--;

	ListNode<T> *nextNode = node->next_;
	delete node;
	return nextNode;
}

template <class T>
ListNode<T> *List<T>::removeRange(ListNode<T> *firstNode, ListNode<T> *lastNode)
{
	// Early-out to prevent `nullptr` dereferencing
	if (firstNode == nullptr)
		return nullptr;

	ListNode<T> *previous = firstNode->previous_;
	ListNode<T> *next = nullptr;
	ListNode<T> *current = firstNode;
	while (current != lastNode)
	{
		next = current->next_;
		delete current;
		size_--;
		current = next;
	}

	if (previous)
		previous->next_ = lastNode;
	else // removing the head
		head_ = lastNode;

	if (lastNode)
		lastNode->previous_ = previous;
	else // removing the tail
		tail_ = previous;

	return lastNode;
}

}

#endif
