#pragma once

namespace bmhpal {
namespace hash {

// 16-byte hash signature
struct BMHPAL_API Sig16 {
	union {
		uint8_t  Bytes[16];
		uint64_t QWords[2];
	};

	Sig16() {
		memset(Bytes, 0, sizeof(Bytes));
	}
	Sig16(const Sig16& b) {
		memcpy(Bytes, b.Bytes, sizeof(Bytes));
	}

	bool        IsNull() const;
	std::string Hex() const;

	bool operator==(const Sig16& b) const {
		return ((QWords[0] ^ b.QWords[0]) |
		        (QWords[1] ^ b.QWords[1])) == 0;
	}
	bool operator!=(const Sig16& b) const {
		return !(*this == b);
	}

	static Sig16 Compute(const void* buf, size_t len);
};

static_assert(sizeof(Sig16) == 16, "Sig16 size");

// Compute a 16-bit hash signature in chunks
class BMHPAL_API Sig16Stream {
public:
	spooky_state State;

	Sig16Stream(uint64_t seed1 = 0, uint64_t seed2 = 0);
	void  Append(const void* buf, size_t len);
	Sig16 Final();
};

} // namespace hash
} // namespace bmhpal

namespace ohash {
template <>
inline hashkey_t gethashcode(const bmhpal::hash::Sig16& s) {
	return (hashkey_t) s.QWords[0];
}
} // namespace ohash
