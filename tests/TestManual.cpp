#include "pch.h"

namespace bmhpal {

// Various manual stuff that doesn't actually have any assertions
TESTFUNC(Manual) {
	tsf::print("HOME: '%v'\n", os::UserHomeDir());
	tsf::print("LocalAppData: '%v'\n", os::UserLocalAppData());
	tsf::print("Seconds since 1970: '%v'\n", (int64_t) time::Now().Minus1970().Seconds64());
	tsf::print("%.1f %.1f\n", time::Now().Minus1970().Seconds(), (time::Now() + time::Second * 5).Minus1970().Seconds());
}
} // namespace bmhpal
