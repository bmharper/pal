#include "pch.h"

using namespace std;

namespace bmhpal {

#ifdef _WIN32
#define PSEP "\\"
#else
#define PSEP "/"
#endif

static void Same(string actual, string expect) {
	if (actual != expect) {
		tsf::print("Expected: '%v'\nActual:  '%v'\n", expect, actual);
	}
	TTASSERT(actual == expect);
}

TESTFUNC(Path) {
	Same(path::Join("", ""), "");
	Same(path::Join("", "b"), "b");
	Same(path::Join("a", ""), "a");
	Same(path::Join("a", "b"), "a" PSEP "b");
	Same(path::Join("/a/", "b"), "/a/b");
	Same(path::Join("/a/", "/b"), "/a/b");
	Same(path::Join("/a", "/b"), "/a/b");
	Same(path::SlashToNative("/a"), PSEP "a");
	Same(path::SlashToNative("\\a"), PSEP "a");
	Same(path::SlashToNative("a\\b"), "a" PSEP "b");
	Same(path::SlashToNative("a/b"), "a" PSEP "b");
}
} // namespace bmhpal