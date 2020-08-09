#pragma once

#include <chrono>
#include "../Math_.h"

namespace bmhpal {
namespace time {

// Nanosecond-precision duration
// To form a duration, multiply an integer by one of the predefined constants Nanosecond, Second, etc.
//   Duration d = 30 * Second;
class BMHPAL_API Duration {
public:
	Duration() {}
	Duration(std::chrono::nanoseconds nsec) : D((int64_t) nsec.count()) {}
	explicit Duration(int64_t nsec) : D(nsec) {}
	Duration operator*(int64_t m) const {
		return Duration(D * m);
	}
	Duration operator/(int32_t dm) const {
		return Duration(D / dm);
	}
	Duration operator+(Duration b) const {
		return Duration(D + b.D);
	}
	Duration operator-(Duration b) const {
		return Duration(D - b.D);
	}
	bool operator==(Duration b) const {
		return D == b.D;
	}
	bool operator!=(Duration b) const {
		return D != b.D;
	}
	bool operator<(Duration b) const {
		return D < b.D;
	}
	bool operator>(Duration b) const {
		return D > b.D;
	}
	bool operator<=(Duration b) const {
		return D <= b.D;
	}
	bool operator>=(Duration b) const {
		return D >= b.D;
	}
	int64_t Nanoseconds() const {
		return D;
	}
	double Microseconds() const {
		return (double) D / 1000.0;
	}
	double Milliseconds() const {
		return (double) D / 1000000.0;
	}
	double Seconds() const {
		return (double) D / 1000000000.0;
	}
	double Minutes() const {
		return (double) D / (1000000000.0 * 60);
	}
	double Hours() const {
		return (double) D / (1000000000.0 * 3600);
	}

	int64_t Microseconds64() const {
		return D / 1000;
	}
	int64_t Milliseconds64() const {
		return D / 1000000;
	}
	int64_t Seconds64() const {
		return D / 1000000000;
	}
	int64_t Minutes64() const {
		return D / (1000000000 * 60ull);
	}
	int64_t Hours64() const {
		return D / (1000000000 * 3600ull);
	}

	std::chrono::nanoseconds Chrono() const {
		return std::chrono::nanoseconds(D);
	}
	operator std::chrono::nanoseconds() const {
		return std::chrono::nanoseconds(D);
	}

private:
	int64_t D = 0;
};

inline Duration operator*(int64_t m, Duration d) {
	return d * m;
}

const Duration Nanosecond(1);
const Duration Microsecond(1000);
const Duration Millisecond(1000000);
const Duration Second(1000000000);
const Duration Minute(60 * (int64_t) 1000000000);
const Duration Hour(3600 * (int64_t) 1000000000);

// Wrapper around std::chrono::system_clock
class BMHPAL_API Time {
public:
	typedef std::chrono::system_clock::time_point TTime;

	TTime T;

	Time() {
		T = TTime();
	}
	Time(TTime t) : T(t) {}

	static Time Now();
	static Time FromUnix(double seconds);

	// I have only tested this on Linux.
	// This cannot represent dates after 2262.
	Duration Minus1970() const {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(T.time_since_epoch());
	}
	double ToUnix() const {
		return Minus1970().Seconds();
	}

	bool IsNull() const {
		return T == TTime();
	}

	bool operator==(const Time& t) const {
		return T == t.T;
	}
	bool operator!=(const Time& t) const {
		return T != t.T;
	}
	bool operator<(const Time& t) const {
		return T < t.T;
	}
	bool operator<=(const Time& t) const {
		return T <= t.T;
	}
	bool operator>(const Time& t) const {
		return T > t.T;
	}
	bool operator>=(const Time& t) const {
		return T >= t.T;
	}

	Time& operator+=(const Duration& d) {
		T += std::chrono::duration_cast<std::chrono::system_clock::duration>(d.Chrono());
		return *this;
	}

	Time& operator-=(const Duration& d) {
		T -= std::chrono::duration_cast<std::chrono::system_clock::duration>(d.Chrono());
		return *this;
	}
};

static Duration operator-(const Time& a, const Time& b) {
	auto d = a.T - b.T;
	return std::chrono::duration_cast<std::chrono::nanoseconds>(d);
}

static Time operator+(const Time& a, const Duration& b) {
	return a.T + std::chrono::duration_cast<std::chrono::system_clock::duration>(b.Chrono());
}

static Time operator-(const Time& a, const Duration& b) {
	return a.T - std::chrono::duration_cast<std::chrono::system_clock::duration>(b.Chrono());
}

BMHPAL_API Time Now();

BMHPAL_API int64_t PerformanceCounter();
BMHPAL_API int64_t PerformanceFrequency();

class BMHPAL_API Benchmark {
public:
	int64_t StartTime = 0;
	Benchmark();
	void   Start();
	double Seconds() const;
	double Milliseconds() const;
	double Nanoseconds() const;
};

class BMHPAL_API Profiler {
public:
	int64_t                         CurStart = 0;
	std::string                     CurItem;
	ohash::map<std::string, double> Seconds;

	void        Start(const char* item);
	void        End();
	void        Reset();
	void        Scale(double x); // Scale all time measurements by x. Useful when you've done 100 repetitions, and you want to multiply by 1/100
	std::string Results(Duration unit, std::string indent = "", double scale = 1) const;
	void        Print(Duration unit, std::string indent = "", double scale = 1) const;
};

class BMHPAL_API ProfileBatch {
public:
	std::vector<Profiler> Samples;

	void                  Add(const Profiler& p);
	std::string           Results(Duration unit, std::string indent = "");
	math::MeanAndVariance ItemStats(std::string item) const;
};

} // namespace time
} // namespace bmhpal
