#pragma once

#include "math.h"

namespace bmhpal {

template <typename T>
T Clamp(T v, T vmin, T vmax) { return (v < vmin) ? vmin : (v > vmax) ? vmax : v; }

template <typename T>
T RoundUp(T v, T mod) { return mod * ((v + mod - 1) / mod); }

namespace math {
struct MeanAndVariance {
	double Mean = 0;
	double Var  = 0;
	double Std  = 0;
};

template <typename T>
MeanAndVariance BasicStats(size_t n, const T* values) {
	double m = 0;
	for (size_t i = 0; i < n; i++)
		m += values[i];
	m /= (double) n;
	double var = 0;
	for (size_t i = 0; i < n; i++)
		var += (values[i] - m) * (values[i] - m);
	MeanAndVariance res;
	res.Mean = m;
	res.Var  = var / n;
	res.Std  = sqrt(res.Var);
	return res;
}

template <typename TContainer>
MeanAndVariance BasicStats(const TContainer& cont) {
	return BasicStats(cont.size(), &cont[0]);
}

} // namespace math
} // namespace bmhpal