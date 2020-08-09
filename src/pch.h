#ifndef BMHPAL_API
#ifdef _WIN32
//#define BMHPAL_API __declspec(dllexport)
#define BMHPAL_API
#else
#define BMHPAL_API
#endif
#endif

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <third_party/utfz/utfz.h>

// When compiling for emscripten, then change our behaviour, so that we can cherry-pick
// select files from the pal library, without having to pull in the world.

#ifndef __EMSCRIPTEN__
#include <curl/curl.h>
#endif

#include "common.h"

#ifdef _WIN32
#include <Userenv.h>
#include <Shlobj.h>
#endif

#include "Error/Asserts.h"

//#include <WinSock2.h>
//#include <third_party/TinyTest/StackWalker/StackWalker.h> // This is for use inside StackTrace.cpp, for crash dump reporting (ie not unit tests)
