#include "pch.h"

using namespace std;

namespace bmhpal {

struct Item {
	int Val;
	Item(int v = 0) : Val(v) {}
};

bool IsItemLessThan(const Item& item, const int& key) {
	return item.Val < key;
}

TESTFUNC(BinarySearch) {
	vector<Item> items;

	auto check = [&](int key, size_t expectResult) {
		size_t findResult = BinarySearchTry(items.size(), &items[0], key, &IsItemLessThan);
		TTASSEQ(findResult, expectResult);
	};

	items = {5, 7, 9};
	check(1, 0);
	check(5, 0); // exists
	check(6, 1);
	check(7, 1); // exists
	check(8, 2);
	check(9, 2); // exists
	check(10, 2);

	items = {5, 5, 7, 9};
	check(6, 2);
}

} // namespace bmhpal
