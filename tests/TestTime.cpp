#include "pch.h"

using namespace std;

namespace bmhpal {

TESTFUNC(Time) {
	{
		double t1 = 1570648756;
		auto   x  = time::Time::FromUnix(t1);
		double t2 = x.ToUnix();
		TTASSEQ(t1, t2);
	}
	{
		double t1 = 1570648756.123;
		auto   x  = time::Time::FromUnix(t1);
		double t2 = x.ToUnix();
		TTASSERT(fabs(t1 - t2) < 0.0001);
	}
}

} // namespace bmhpal
