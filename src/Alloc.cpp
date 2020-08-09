#include "pch.h"

namespace bmhpal {
BMHPAL_API void* malloc_or_die(size_t len) {
	void* buf = malloc(len);
	if (!buf)
		BMHPAL_DIE_MSG("Out of memory");
	return buf;
}

BMHPAL_API void* realloc_or_die(void* buf, size_t len) {
	void* nbuf = realloc(buf, len);
	if (!nbuf) {
		BMHPAL_DIE_MSG("Out of memory");
		return nullptr;
	}
	return nbuf;
}
} // namespace bmhpal