#pragma once

#include <mutex>
#include "../Sync/sema.h"

namespace bmhpal {

/*

	Work/job queue
	==============

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
class BMHPAL_API Queue {
public:
	typedef bool (*ScanCallback)(void* context, void* item);

	std::mutex Lock;      // Lock on the queue data structures
	Semaphore  Semaphore; // Can be used to wait for detection of a non-empty queue. Only valid if semaphore was enabled during call to Initialize(). Read CAVEAT.

	Queue();
	~Queue();

	void   Initialize(bool useSemaphore, size_t itemSize); // Every item must be the same size
	void   Push(const void* item);                         // Add to head. We copy in itemSize bytes, from base address 'item'
	bool   PopTail(void* item);                            // Pop the tail of the queue. Returns false if the queue is empty.
	bool   PeekTail(void* item);                           // Get the tail of the queue, but do not pop it. Obviously useless for multithreaded scenarios, unless you have acquired the lock.
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
	size_t ItemSize;
	void*  Buffer;

	size_t Mask() const {
		return RingSize - 1;
	}
	void* Slot(size_t pos) const {
		return (uint8_t*) Buffer + (pos * ItemSize);
	}
	void Increment(size_t& i) const {
		i = (i + 1) & Mask();
	}
	size_t SizeInternal() const {
		return (Head - Tail) & Mask();
	}
	void Grow();
};

// Typed wrapper around Queue
template <typename T>
class TQueue {
public:
	TQueue() {
		Q.Initialize(false, sizeof(T));
	}
	void Initialize(bool useSemaphore) {
		Q.Initialize(useSemaphore, sizeof(T));
	}
	void Push(const T& item) {
		Q.Push(&item);
	}
	bool PopTail(T& item) {
		return Q.PopTail(&item);
	}
	bool PeekTail(T& item) {
		return Q.PeekTail(&item);
	}
	T PopTailR() {
		T t = T();
		PopTail(t);
		return t;
	}
	T PeekTailR() {
		T t = T();
		PeekTail(t);
		return t;
	}
	size_t Size() {
		return Q.Size();
	}
	std::mutex& LockObj() {
		return Q.Lock;
	}
	Semaphore& SemaphoreObj() {
		return Q.Semaphore;
	}
	void Scan(bool forwards, void* context, bool (*cb)(void* context, T* item)) {
		Q.Scan(forwards, context, (Queue::ScanCallback) cb);
	}

private:
	Queue Q;
};
} // namespace bmhpal