#include "pch.h"

using namespace std;

namespace bmhpal {

static void TestQueueSequence(ObjQueue<std::string>& q, vector<string> items) {
	for (auto s : items)
		q.Push(s);
	for (size_t i = 0; i < items.size(); i++) {
		string x;
		bool   ok = q.PopTail(x);
		TTASSERT(ok);
		TTASSEQ(x, items[i]);
	}
}

TESTFUNC(Queue) {
	{
		ObjQueue<std::string> q;
		q.Initialize(false);
		TestQueueSequence(q, {"a", "b", "c"});
		// this second sequence triggers a vital codepath, which is growing the ring when Head is behind Tail
		TestQueueSequence(q, {"1", "2", "3", "4"});
		// and why not just add some more...
		TestQueueSequence(q, {"x", "y", "z"});
	}
}

} // namespace bmhpal
