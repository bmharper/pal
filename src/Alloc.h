#pragma once

namespace bmhpal {
BMHPAL_API void* malloc_or_die(size_t len);
BMHPAL_API void* realloc_or_die(void* buf, size_t len);
} // namespace bmhpal