#include "../Error/Asserts.h"

namespace bmhpal {
namespace algo {

enum class Filters {
	Min,
	Max,
};

// Returns either filtered or index, or both.
// Both filtered and index are the same size as 'in'.
// filtered contains the filtered values (eg min/max)
// index contains the index of the item that was the min/max
template <typename T>
void MinMaxFilter(Filters f, const std::vector<T>& in, int size, std::vector<T>* filtered, std::vector<size_t>* index) {
	// filter size must be an odd number (eg 1, 3, 5, 7)
	BMHPAL_ASSERT((size - 1) % 2 == 0);
	int symSize = (size - 1) / 2;

	if (filtered)
		filtered->resize(in.size());
	if (index)
		index->resize(in.size());
	ssize_t n = (ssize_t) in.size();
	if (n == 0)
		return;

	for (ssize_t i = 0; i < n; i++) {
		ssize_t a   = std::max<ssize_t>(0, i - symSize);
		ssize_t b   = std::min<ssize_t>(n, i + symSize + 1);
		ssize_t idx = a;
		T       v   = in[a];
		switch (f) {
		case Filters::Min:
			for (auto j = a; j < b; j++) {
				if (in[j] < v) {
					v   = in[j];
					idx = j;
				}
			}
			break;
		case Filters::Max:
			for (auto j = a; j < b; j++) {
				if (in[j] > v) {
					v   = in[j];
					idx = j;
				}
			}
			break;
		}
		if (filtered)
			(*filtered)[i] = v;
		if (index)
			(*index)[i] = (size_t) idx;
	}
}
} // namespace algo
} // namespace bmhpal