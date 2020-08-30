#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Found this in the Chrome sources, via a PVS studio blog post
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
typedef SSIZE_T ssize_t;
#endif

#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <atomic>

#include <third_party/tsf/tsf.h>
#include <third_party/utfz/utfz.h>
#include <third_party/CxxUrl/url.hpp>
#include <third_party/xxHash/xxhash.h>

#ifndef __EMSCRIPTEN__
#include <openssl/sha.h>
#include <openssl/rand.h>
#endif

#include <third_party/modp/modp_b64.h>
#include <third_party/modp/modp_b16.h>
#include <third_party/modp/modp_burl.h>

#ifdef _MSC_VER
// Unfortunately this warning only comes out at template instantiation time, so we can't push/pop it.
// The warning (4503) is about decorator name being too long, so it's benign enough.
#pragma warning(disable : 4503)
#endif
#include <third_party/nlohmann-json/json.hpp>

#include <third_party/spooky/spooky.h>

#include <third_party/ohash/ohashmap.h>
#include <third_party/ohash/ohashset.h>
