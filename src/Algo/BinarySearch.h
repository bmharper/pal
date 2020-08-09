#pragma once

namespace bmhpal {

// Binary Search, but always return stopping position, regardless of match
// * This will walk to the first in a series of matches
// A typical signature for lessThan is "bool lessThan(const TItem& item, const TKey& key)"
template <typename TItem, typename TKey, typename TLessThan>
size_t BinarySearchTry(size_t n, const TItem* items, const TKey& key, TLessThan lessThan) {
	if (n == 0) {
		// This is the only case where we return an invalid index
		return -1;
	}
	size_t imin = 0;
	size_t imax = n;
	while (imax > imin) {
		size_t imid = (imin + imax) / 2;
		if (lessThan(items[imid], key))
			imin = imid + 1;
		else
			imax = imid;
	}
	if (imin == n) {
		// This makes the case:
		//  key > [all items]
		//   consistent with
		//  key < [all items]
		// so that we always return a valid index.
		return n - 1;
	}
	return imin;
}

} // namespace bmhpal