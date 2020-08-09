#pragma once

// Tools for data visualization

namespace bmhpal {
namespace viz {

enum class Flags {
	None = 0,
	LogY = 1,
};

inline Flags    operator|(Flags a, Flags b) { return Flags((uint32_t) a | (uint32_t) b); }
inline uint32_t operator&(Flags a, Flags b) { return (uint32_t) a & (uint32_t) b; }

std::string Dir();
std::string Filename(std::string name);

void DumpTimeSeriesD(const std::vector<std::pair<double, double>>& ts, Flags flags, std::string title);

template <typename T>
void DumpTimeSeries(const std::vector<T>& ts, Flags flags = Flags::None, std::string title = "timeseries") {
	std::vector<std::pair<double, double>> pairs;
	for (size_t i = 0; i < ts.size(); i++)
		pairs.emplace_back((double) i, (double) ts[i]);
	DumpTimeSeriesD(pairs, flags, title);
}

template <typename T1, typename T2>
void DumpTimeSeries(const std::vector<std::pair<T1, T2>>& ts, Flags flags = Flags::None, std::string title = "timeseries") {
	std::vector<std::pair<double, double>> pairs;
	for (size_t i = 0; i < ts.size(); i++)
		pairs.emplace_back((double) ts[i].first, (double) ts[i].second);
	DumpTimeSeriesD(pairs, flags, title);
}

} // namespace viz
} // namespace bmhpal