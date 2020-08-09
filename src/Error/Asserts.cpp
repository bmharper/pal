// EMSCRIPTEN
#include "pch.h"
#include "Asserts.h"
#include "StackTrace.h"

namespace bmhpal {

BMHPAL_API void BMHPAL_NORETURN Die(const char* file, int line, const char* msg) {
#if BMHPAL_PLATFORM_ANDROID
	__android_log_print(ANDROID_LOG_ERROR, "bmhpal", "assertion failed %s:%d %s", file, line, msg);
#endif
	fprintf(stdout, "Program is self-terminating at %s:%d. (%s)\n", file, line, msg);
	fprintf(stderr, "Program is self-terminating at %s:%d. (%s)\n", file, line, msg);
	PrintStackTrace();
	fflush(stdout);
	fflush(stderr);
#ifdef BMHPAL_PLATFORM_WINDOWS
	auto s = tsf::fmt("Program is self-terminating at %s:%d. (%s)", file, line, msg);
	MessageBoxA(nullptr, s.c_str(), "Internal Error", MB_OK);
#endif
	BMHPAL_DEBUG_BREAK();
	*((int*) 0) = 1;
}

} // namespace bmhpal