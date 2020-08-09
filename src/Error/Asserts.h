#pragma once

#include "../PlatformDefinitions.h"

namespace bmhpal {

BMHPAL_API void BMHPAL_NORETURN Die(const char* file, int line, const char* msg);

#define BMHPAL_DIE() bmhpal::Die(__FILE__, __LINE__, "")
#define BMHPAL_DIE_MSG(msg) bmhpal::Die(__FILE__, __LINE__, msg)

// NOTE: This is compiled in all builds (Debug, Release)
#define BMHPAL_ASSERT(f) (void) ((f) || (bmhpal::Die(__FILE__, __LINE__, #f), 0))

#ifdef _DEBUG
#define BMHPAL_VERIFY(x) BMHPAL_ASSERT(x)
#define BMHPAL_DEBUG_ASSERT(x) BMHPAL_ASSERT(x)
#else
#define BMHPAL_VERIFY(x) ((void) (x))
#define BMHPAL_DEBUG_ASSERT(x) ((void) 0)
#endif

#define BMHPAL_TODO BMHPAL_DIE_MSG("not yet implemented")
#define BMHPAL_TODO_STATIC static_assert(false, "Implement me");
} // namespace bmhpal