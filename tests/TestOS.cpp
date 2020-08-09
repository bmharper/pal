#include "pch.h"

using namespace std;

namespace bmhpal {

int AntiOptimizer;

TESTFUNC(OS) {
	const string filename = "junk.test";
	const string content  = "123 abc";

	auto err = os::WriteFile(filename, content);
	TTASSERT(err.OK());
	string buf;

	err = os::ReadFile(filename, buf, os::ReadFlags::Stream);
	TTASSERT(err.OK());
	TTASSERT(buf == content);
	buf = "";

	err = os::ReadFile(filename, buf, os::ReadFlags::None);
	TTASSERT(err.OK());
	TTASSERT(buf == content);

	err = os::Remove(filename);
	TTASSERT(err.OK());

	err = os::RemoveAll("/a_bogus_path/that_should_not.exist");
	TTASSERT(err.OK());

	os::FileAttributes attribs;
	err = os::Stat(filename, attribs);
	TTASSERT(os::IsNotExist(err));

#ifdef BMHPAL_PLATFORM_LINUX
	int n = 1000;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			AntiOptimizer = AntiOptimizer * 2 + 1;
		}
	}
	// Adding this in helps verify that our child processes also count against our time.
	// On my machine right now, this adds about 500ms to our time
	//system("find /usr *.h 1>nul");

	double self     = 0;
	double children = 0;
	err             = os::CPUTime(self, children);
	TTASSERT(err.OK());
	tsf::print("cpu time. self: %f ms, children: %f ms\n", self * 1000.0, children * 1000.0);
// If both self time and child find time is included, expect to see about 5ms and 500ms for self, children, respectively.
#endif
}
} // namespace bmhpal