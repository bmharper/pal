#pragma once

namespace bmhpal {
namespace hash {

// 32-byte hash signature
struct BMHPAL_API Sig32 {
	union {
		uint8_t  Bytes[32];
		uint64_t QWords[4];
	};

	Sig32() {
		memset(Bytes, 0, sizeof(Bytes));
	}
	Sig32(const Sig32& b) {
		memcpy(Bytes, b.Bytes, sizeof(Bytes));
	}

	bool        IsNull() const;
	std::string Hex() const;

	static Sig32 Compute(const void* buf, size_t len);

	// Might as well do a constant-time compare
	bool operator==(const Sig32& b) const {
		return ((QWords[0] ^ b.QWords[0]) |
		        (QWords[1] ^ b.QWords[1]) |
		        (QWords[2] ^ b.QWords[2]) |
		        (QWords[3] ^ b.QWords[3])) == 0;
	}
	bool operator!=(const Sig32& b) const {
		return !(*this == b);
	}
};

static_assert(sizeof(Sig32) == 32, "Sig32 size");

// Compute a 32-bit hash signature in chunks
class BMHPAL_API Sig32Stream {
public:
	SHA256_CTX CX;

	Sig32Stream();
	void  Append(const void* buf, size_t len);
	Sig32 Final();
};

} // namespace hash
} // namespace bmhpal
