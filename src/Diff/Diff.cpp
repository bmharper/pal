#include "pch.h"
#include "Diff.h"

using namespace std;

namespace bmhpal {
namespace diff {

BMHPAL_API size_t StringDistance(const std::string& a, const std::string& b, size_t* _nDelete, size_t* _nInsert) {
	std::string r       = a;
	size_t      nDelete = 0;
	size_t      nInsert = 0;

	auto patch = [&](PatchOp op, size_t pos, size_t len, const char* el) {
		if (op == PatchOp::Delete) {
			nDelete += len;
		} else if (op == PatchOp::Insert) {
			nInsert += len;
		}
	};

	DiffCore   d;
	CharTraits traits;
	d.Diff<char, CharTraits>(a.size(), b.size(), a.c_str(), b.c_str(), traits, patch);
	if (_nDelete)
		*_nDelete = nDelete;
	if (_nInsert)
		*_nInsert = nInsert;
	return nDelete + nInsert;
}

} // namespace diff
} // namespace bmhpal
