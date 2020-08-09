// EMSCRIPTEN
#include "pch.h"

#include "Time_.h"
#include "../Math_.h"

using namespace std;

namespace bmhpal {
namespace time {

Time Time::Now() {
	Time t;
	t.T = std::chrono::system_clock::now();
	return t;
}

Time Time::FromUnix(double seconds) {
	int64_t ival = (int64_t) floor(seconds);
	Time    t    = std::chrono::system_clock::from_time_t((time_t) ival);
	t += (int64_t) floor((seconds - (double) ival) * 1e9) * Nanosecond;
	return t;
}

BMHPAL_API Time Now() {
	return Time::Now();
}

BMHPAL_API int64_t PerformanceCounter() {
#ifdef _WIN32
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return t.QuadPart;
#else
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (uint64_t) t.tv_sec * (uint64_t) 1000000000 + t.tv_nsec;
#endif
}

BMHPAL_API int64_t PerformanceFrequency() {
#ifdef _WIN32
	LARGE_INTEGER f;
	QueryPerformanceFrequency(&f);
	return f.QuadPart;
#else
	return 1000000000;
#endif
}

Benchmark::Benchmark() {
	Start();
}

void Benchmark::Start() {
	StartTime = PerformanceCounter();
}

double Benchmark::Seconds() const {
	return (double) (PerformanceCounter() - StartTime) / (double) PerformanceFrequency();
}

double Benchmark::Milliseconds() const {
	return 1000.0 * (double) (PerformanceCounter() - StartTime) / (double) PerformanceFrequency();
}

double Benchmark::Nanoseconds() const {
	return 1000000000.0 * (double) (PerformanceCounter() - StartTime) / (double) PerformanceFrequency();
}

void Profiler::Start(const char* item) {
	End();
	CurItem  = item;
	CurStart = PerformanceCounter();
}

void Profiler::End() {
	if (CurItem.size() == 0)
		return;
	double seconds = (double) (PerformanceCounter() - CurStart) / (double) PerformanceFrequency();
	if (Seconds.contains(CurItem)) {
		Seconds[CurItem] += seconds;
	} else {
		Seconds[CurItem] = seconds;
	}
}

void Profiler::Reset() {
	*this = Profiler();
}

static double FormatProfileUnit(Duration unit, const char*& unitName) {
	double scale = 0;
	switch (unit.Nanoseconds()) {
	case 1:
		scale    = 1e9;
		unitName = "ns";
		break;
	case 1000:
		scale    = 1e6;
		unitName = "us";
		break;
	case 1000000:
		scale    = 1e3;
		unitName = "ms";
		break;
	case 1000000000:
		scale    = 1;
		unitName = "s";
		break;
	default:
		// illegal time unit
		BMHPAL_ASSERT(false);
	}
	return scale;
}

void Profiler::Scale(double x) {
	for (auto& p : Seconds)
		p.second *= x;
}

std::string Profiler::Results(Duration unit, std::string indent, double scale) const {
	vector<pair<double, string>> all;
	for (const auto& p : Seconds)
		all.emplace_back(p.second, p.first);
	sort(all.begin(), all.end());
	std::reverse(all.begin(), all.end());
	string r;
	for (const auto& p : all) {
		const char* unitName = nullptr;
		double      v        = p.first * scale * FormatProfileUnit(unit, unitName);
		r += tsf::fmt("%s%20s: %.2f %v\n", indent, p.second, v, unitName);
	}
	return r;
}

void Profiler::Print(Duration unit, std::string indent, double scale) const {
	tsf::print("%v", Results(unit, indent, scale));
}

void ProfileBatch::Add(const Profiler& p) {
	Samples.push_back(p);
}

std::string ProfileBatch::Results(Duration unit, std::string indent) {
	// Create a fake new entry called Total
	for (auto& s : Samples) {
		if (s.Seconds.contains("<Total>"))
			continue;
		double total = 0;
		for (const auto& p : s.Seconds)
			total += p.second;
		s.Seconds["<Total>"] = total;
	}

	// Gather all unique keys, and also compute the total
	// time spent for each key, so that we can sort from most expensive to least expensive.
	ohash::set<string>         keys;
	ohash::map<string, double> sums;
	for (const auto& s : Samples) {
		double total = 0;
		for (const auto& p : s.Seconds) {
			sums[p.first] += p.second;
			keys.insert(p.first);
			total += p.second;
		}
	}

	vector<pair<double, string>> all;
	for (const auto& p : sums)
		all.emplace_back(p.second, p.first);
	std::sort(all.begin(), all.end());
	std::reverse(all.begin(), all.end());

	std::string r;
	for (const auto& var : all) {
		vector<double> x;
		for (const auto& s : Samples) {
			double* v = s.Seconds.getp(var.second);
			if (!v)
				continue;
			x.push_back(*v);
		}

		const char* unitName = nullptr;
		double      scale    = FormatProfileUnit(unit, unitName);

		if (x.size() >= 2) {
			auto stats = math::BasicStats(x);
			r += tsf::fmt("%s%20s: %.1f %s (± %.1f)\n", indent, var.second, scale * stats.Mean, unitName, scale * stats.Std);
		} else {
			r += tsf::fmt("%s%20s: %.1f %s (1 sample)\n", indent, var.second, scale * x[0], unitName);
		}
	}
	return r;
}

math::MeanAndVariance ProfileBatch::ItemStats(std::string item) const {
	vector<double> all;
	for (const auto& s : Samples) {
		if (s.Seconds.contains(item))
			all.push_back(s.Seconds.get(item));
	}
	if (all.size() == 0) {
		math::MeanAndVariance s;
		s.Mean = 9e99;
		s.Std  = 9e99;
		s.Var  = 9e99;
		return s;
	}
	return math::BasicStats(all);
}

} // namespace time
} // namespace bmhpal