#include "pch.h"
#include "Sig32.h"

namespace bmhpal {
namespace hash {

bool Sig32::IsNull() const {
	for (int i = 0; i < arraysize(QWords); i++) {
		if (QWords[i] != 0)
			return false;
	}
	return true;
}

std::string Sig32::Hex() const {
	return modp::b16_encode((const char*) Bytes, sizeof(Bytes));
}

Sig32 Sig32::Compute(const void* buf, size_t len) {
	Sig32 s;
	SHA256((const unsigned char*) buf, len, (unsigned char*) s.Bytes);
	return s;
}

Sig32Stream::Sig32Stream() {
	SHA256_Init(&CX);
}

void Sig32Stream::Append(const void* buf, size_t len) {
	SHA256_Update(&CX, buf, len);
}

Sig32 Sig32Stream::Final() {
	Sig32 s;
	SHA256_Final((unsigned char*) s.Bytes, &CX);
	return s;
}

} // namespace hash
} // namespace bmhpal