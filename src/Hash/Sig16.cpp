#include "pch.h"
#include "Sig16.h"

namespace bmhpal {
namespace hash {

bool Sig16::IsNull() const {
	for (int i = 0; i < arraysize(QWords); i++) {
		if (QWords[i] != 0)
			return false;
	}
	return true;
}

std::string Sig16::Hex() const {
	return modp::b16_encode((const char*) Bytes, sizeof(Bytes));
}

Sig16 Sig16::Compute(const void* buf, size_t len) {
	Sig16 s;
	spooky_hash128(buf, len, &s.QWords[0], &s.QWords[1]);
	return s;
}

Sig16Stream::Sig16Stream(uint64_t seed1, uint64_t seed2) {
	spooky_init(&State, seed1, seed2);
}

void Sig16Stream::Append(const void* buf, size_t len) {
	spooky_update(&State, buf, len);
}

Sig16 Sig16Stream::Final() {
	Sig16 s;
	spooky_final(&State, &s.QWords[0], &s.QWords[1]);
	return s;
}

} // namespace hash
} // namespace bmhpal