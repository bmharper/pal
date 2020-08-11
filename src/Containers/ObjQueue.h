#pragma once

#include <mutex>
#include <algorithm>
#include <string.h>
#include "../Sync/sema.h"

namespace bmhpal {

/*

	Work/job queue
	==============

	This differs from 'Queue' in that it's not an opaque block of memory, but always templated,
	so it's safe to use with any C++ object.

	* Multithreaded
	* Simple FIFO
	* Ring buffer

	We follow ryg's recommendations here from http://fgiesen.wordpress.com/2010/12/14/ring-buffers-and-queues/
	Particularly, ring size is always a power of 2, and we use at most N-1 slots. This removes the ambiguity caused
	by a full buffer, wherein Head = Tail, which is the same as an empty buffer.

	CAVEAT!

	If you choose to use the semaphore, then ALL of your queue consumers MUST obey this pattern:
		1 Wait for the semaphore to be signaled
		2 Fetch one item from the queue
		3 Go back to (1)

	*/
template <typename T>
class BMHPAL_API ObjQueue {
public:
	typedef bool (*ScanCallback)(void* context, T& item);

	std::mutex Lock;      // Lock on the queue data structures
	Semaphore  Semaphore; // Can be used to wait for detection of a non-empty queue. Only valid if semaphore was enabled during call to Initialize(). Read CAVEAT.

	ObjQueue();
	~ObjQueue();

	void   Initialize(bool useSemaphore);
	void   Push(const T& item); // Add to head. We copy in itemSize bytes, from base address 'item'
	bool   PopTail(T& item);    // Pop the tail of the queue. Returns false if the queue is empty.
	bool   PeekTail(T& item);   // Get the tail of the queue, but do not pop it. Obviously useless for multithreaded scenarios, unless you have acquired the lock.
	size_t Size();

	// Scan through the queue, allowing you to mutate items inside the queue.
	// The callback function 'cb' is called once for every item in the queue.
	// If forwards is true, then we iterate from Tail to Head.
	// If forwards is false, then we iterate from Head to Tail.
	// Return false from your iterator function to end the scan prematurely.
	void Scan(bool forwards, void* context, ScanCallback cb);

private:
	bool   HaveSemaphore;
	size_t Tail;
	size_t Head;
	size_t RingSize; // Size of the ring buffer. Always a power of 2.
	T*     Buffer;

	size_t Mask() const {
		return RingSize - 1;
	}
	void Increment(size_t& i) const {
		i = (i + 1) & Mask();
	}
	size_t SizeInternal() const {
		return (Head - Tail) & Mask();
	}
	void Grow();
};

template <typename T>
ObjQueue<T>::ObjQueue() {
	Tail          = 0;
	Head          = 0;
	RingSize      = 0;
	Buffer        = nullptr;
	HaveSemaphore = false;
}

template <typename T>
ObjQueue<T>::~ObjQueue() {
	delete[] Buffer;
}

template <typename T>
void ObjQueue<T>::Initialize(bool useSemaphore) {
	BMHPAL_ASSERT(SizeInternal() == 0);
	BMHPAL_ASSERT(!HaveSemaphore);
	if (useSemaphore) {
		HaveSemaphore = true;
	}
}

template <typename T>
void ObjQueue<T>::Push(const T& item) {
	std::lock_guard<std::mutex> lock(Lock);

	if (SizeInternal() + 1 >= RingSize)
		Grow();

	Buffer[Head] = item;
	Increment(Head);

	if (HaveSemaphore)
		Semaphore.signal(1);
}

template <typename T>
bool ObjQueue<T>::PopTail(T& item) {
	std::lock_guard<std::mutex> lock(Lock);
	if (SizeInternal() == 0)
		return false;
	item = Buffer[Tail];
	Increment(Tail);
	return true;
}

template <typename T>
bool ObjQueue<T>::PeekTail(T& item) {
	std::lock_guard<std::mutex> lock(Lock);
	if (SizeInternal() == 0)
		return false;
	item = Buffer[Tail];
	return true;
}

template <typename T>
void ObjQueue<T>::Grow() {
	size_t newsize = std::max(RingSize * 2, (size_t) 2);
	T*     nb      = new T[newsize];
	BMHPAL_ASSERT(nb != nullptr);
	for (size_t i = 0; i < RingSize; i++)
		nb[i] = std::move(Buffer[i]);
	delete[] Buffer;
	Buffer = nb;
	// If head is behind tail, then we need to copy the later items in front of the earlier ones.
	if (Head < Tail) {
		for (size_t i = 0; i < Head; i++)
			Buffer[RingSize + i] = std::move(Buffer[i]);
		Head = RingSize + Head;
	}
	RingSize = newsize;
}

template <typename T>
size_t ObjQueue<T>::Size() {
	std::lock_guard<std::mutex> lock(Lock);
	return SizeInternal();
}

template <typename T>
void ObjQueue<T>::Scan(bool forwards, void* context, ScanCallback cb) {
	std::lock_guard<std::mutex> lock(Lock);
	if (forwards) {
		for (size_t i = Head; i != Tail; i = (i - 1) & Mask()) {
			if (!cb(context, Buffer[i]))
				return;
		}
	} else {
		for (size_t i = Tail; i != Head; i = (i + 1) & Mask()) {
			if (!cb(context, Buffer[i]))
				return;
		}
	}
}
} // namespace bmhpal