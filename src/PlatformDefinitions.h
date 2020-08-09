#pragma once

#if defined(_WIN32)

#define BMHPAL_PLATFORM_WINDOWS
#define BMHPAL_TRACE_WRITE OutputDebugStringA
#define BMHPAL_ANALYSIS_ASSUME(x) __analysis_assume(x)
#define BMHPAL_NORETURN __declspec(noreturn)
#define BMHPAL_DEBUG_BREAK() __debugbreak()

#elif defined(ANDROID)

#define BMHPAL_PLATFORM_ANDROID
#define BMHPAL_TRACE_WRITE(msg) __android_log_write(ANDROID_LOG_INFO, "bmhpal", msg)
#define BMHPAL_ANALYSIS_ASSUME(x) ((void) 0)
#define BMHPAL_NORETURN __attribute__((noreturn)) __attribute__((analyzer_noreturn))
#define BMHPAL_DEBUG_BREAK() __builtin_trap()

#elif defined(__linux__)

#define BMHPAL_PLATFORM_LINUX
#define BMHPAL_TRACE_WRITE(msg) fputs(msg, stderr)
#define BMHPAL_ANALYSIS_ASSUME(x) ((void) 0)
#define BMHPAL_NORETURN __attribute__((noreturn)) __attribute__((analyzer_noreturn))
#define BMHPAL_DEBUG_BREAK() __builtin_trap()

#elif defined(__APPLE__)

#define BMHPAL_PLATFORM_APPLE
#define BMHPAL_TRACE_WRITE(msg) fputs(msg, stderr)
#define BMHPAL_ANALYSIS_ASSUME(x) ((void) 0)
#define BMHPAL_NORETURN __attribute__((noreturn)) __attribute__((analyzer_noreturn))
#define BMHPAL_DEBUG_BREAK() __builtin_trap()

#elif defined(__EMSCRIPTEN__)

#define BMHPAL_PLATFORM_WASM
#define BMHPAL_TRACE_WRITE(msg) fputs(msg, stderr)
#define BMHPAL_ANALYSIS_ASSUME(x) ((void) 0)
#define BMHPAL_NORETURN __attribute__((noreturn)) __attribute__((analyzer_noreturn))
#define BMHPAL_DEBUG_BREAK() __builtin_trap()

#endif