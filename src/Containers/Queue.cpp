#include "pch.h"
#include "Queue.h"
#include <algorithm>
#include <string.h>

namespace bmhpal {

Queue::Queue() {
	Tail          = 0;
	Head          = 0;
	RingSize      = 0;
	ItemSize      = 0;
	Buffer        = nullptr;
	HaveSemaphore = false;
}

Queue::~Queue() {
	free(Buffer);
}

void Queue::Initialize(bool useSemaphore, size_t itemSize) {
	BMHPAL_ASSERT(SizeInternal() == 0);
	if (itemSize != this->ItemSize) {
		BMHPAL_ASSERT(this->ItemSize == 0);
		BMHPAL_ASSERT(itemSize > 0);
		BMHPAL_ASSERT(itemSize < 0xffff); // sanity
		ItemSize = itemSize;
	}
	BMHPAL_ASSERT(!HaveSemaphore);
	if (useSemaphore) {
		HaveSemaphore = true;
	}
}

void Queue::Push(const void* item) {
	std::lock_guard<std::mutex> lock(Lock);

	if (SizeInternal() + 1 >= RingSize)
		Grow();

	memcpy(Slot(Head), item, ItemSize);
	Increment(Head);

	if (HaveSemaphore)
		Semaphore.signal(1);
}

bool Queue::PopTail(void* item) {
	std::lock_guard<std::mutex> lock(Lock);
	if (SizeInternal() == 0)
		return false;
	memcpy(item, Slot(Tail), ItemSize);
	Increment(Tail);
	return true;
}

bool Queue::PeekTail(void* item) {
	std::lock_guard<std::mutex> lock(Lock);
	if (SizeInternal() == 0)
		return false;
	memcpy(item, Slot(Tail), ItemSize);
	return true;
}

void Queue::Grow() {
	size_t newsize = std::max(RingSize * 2, (size_t) 2);
	void*  nb      = realloc(Buffer, ItemSize * newsize);
	BMHPAL_ASSERT(nb != nullptr);
	Buffer = nb;
	// If head is behind tail, then we need to copy the later items in front of the earlier ones.
	if (Head < Tail) {
		memcpy(Slot(RingSize), Slot(0), ItemSize * Head);
		Head = RingSize + Head;
	}
	RingSize = newsize;
}

size_t Queue::Size() {
	std::lock_guard<std::mutex> lock(Lock);
	return SizeInternal();
}

void Queue::Scan(bool forwards, void* context, ScanCallback cb) {
	std::lock_guard<std::mutex> lock(Lock);
	if (forwards) {
		for (size_t i = Head; i != Tail; i = (i - 1) & Mask()) {
			if (!cb(context, Slot(i)))
				return;
		}
	} else {
		for (size_t i = Tail; i != Head; i = (i + 1) & Mask()) {
			if (!cb(context, Slot(i)))
				return;
		}
	}
}
} // namespace bmhpal