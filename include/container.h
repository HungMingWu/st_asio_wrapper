/*
 * container.h
 *
 *  Created on: 2016-10-12
 *      Author: youngwolf
 *		email: mail2tao@163.com
 *		QQ: 676218192
 *		Community on QQ: 198941541
 *
 * containers.
 */

#ifndef ST_ASIO_CONTAINER_H_
#define ST_ASIO_CONTAINER_H_

#include "base.h"

namespace st_asio_wrapper
{

//st_asio_wrapper requires that container must take one and only one template argument.
template<typename T> class list : public boost::container::list<T> {};

class dummy_lockable
{
public:
	//lockable, dummy
	bool is_lockable() const {return false;}
	void lock() const {}
	void unlock() const {}
};

class lockable
{
public:
	//lockable
	bool is_lockable() const {return true;}
	void lock() {mutex.lock();}
	void unlock() {mutex.unlock();}

private:
	std::mutex mutex; //std::mutex is more efficient than boost::shared_mutex
};

//Container must at least has the following functions (like list):
// Container() and Container(size_t) constructor
// size (must be thread safe, but doesn't have to be consistent, std::list before gcc 5 doesn't meet this requirement, boost::container::list does)
// empty (must be thread safe, but doesn't have to be consistent)
// clear
// swap
// emplace_back(const T& item)
// emplace_back()
// splice(decltype(Container::end()), boost::container::list<T>&), after this, boost::container::list<T> must be empty
// front
// back
// pop_front
// end
template<typename T, typename Container, typename Lockable> //thread safety depends on Container or Lockable
#ifdef ST_ASIO_DISPATCH_BATCH_MSG
class queue : public Container, public Lockable
#else
class queue : protected Container, public Lockable
#endif
{
public:
	typedef T data_type;

	queue() {}
	queue(size_t capacity) : Container(capacity) {}

	bool is_thread_safe() const {return Lockable::is_lockable();}
	using Container::size;
	using Container::empty;
	void clear() {
		std::lock_guard lock(*this);
		Container::clear();
	}
	void swap(Container& other) {
		std::lock_guard lock(*this);
		Container::swap(other);
	}

	//thread safe
	bool enqueue(const T& item) {
		std::lock_guard lock(*this);
		return enqueue_(item);
	}
	bool enqueue(T& item) {
		std::lock_guard lock(*this);
		return enqueue_(item);
	}
	void move_items_in(boost::container::list<T>& can) {
		std::lock_guard lock(*this);
		move_items_in_(can);
	}
	bool try_dequeue(T& item) {
		std::lock_guard lock(*this);
		return try_dequeue_(item);
	}

	//not thread safe
	bool enqueue_(const T& item)
		{try {emplace_back(item);} catch (const std::exception& e) {unified_out::error_out("cannot hold more objects (%s)", e.what()); return false;} return true;}
	bool enqueue_(T& item) //after this, item will becomes empty, please note.
		{try {ST_THIS emplace_back();} catch (const std::exception& e) {unified_out::error_out("cannot hold more objects (%s)", e.what()); return false;} ST_THIS back().swap(item); return true;}
	void move_items_in_(boost::container::list<T>& can) {ST_THIS splice(ST_THIS end(), can);}
	bool try_dequeue_(T& item) {if (ST_THIS empty()) return false; item.swap(ST_THIS front()); ST_THIS pop_front(); return true;}
};

template<typename T, typename Container> class non_lock_queue : public queue<T, Container, dummy_lockable> //thread safety depends on Container
{
public:
	non_lock_queue() {}
	non_lock_queue(size_t capacity) : queue<T, Container, dummy_lockable>(capacity) {}
};
template<typename T, typename Container> class lock_queue : public queue<T, Container, lockable>
{
public:
	lock_queue() {}
	lock_queue(size_t capacity) : queue<T, Container, lockable>(capacity) {}
};

} //namespace

#endif /* ST_ASIO_CONTAINER_H_ */
