#include "pch.h"
#include "Hex.h"

using namespace std;

namespace bmhpal {
namespace hex {

static const char* EncodeTable = "0123456789abcdef";

BMHPAL_API unsigned
DecodeChar(char hex) {
	if (hex >= '0' && hex <= '9')
		return hex - '0';
	else if (hex >= 'a' && hex <= 'f')
		return 10 + hex - 'a';
	else if (hex >= 'A' && hex <= 'F')
		return 10 + hex - 'A';
	else
		return -1;
}

BMHPAL_API Error Decode(const char* hex, void* _out, size_t outBufferSize) {
	uint8_t* out = (uint8_t*) _out;
	uint8_t* end = out + outBufferSize;
	for (size_t i = 0; hex[i]; i += 2) {
		if (out >= end)
			return Error::Fmt("Out of space decoding hex string");
		unsigned c1 = DecodeChar(hex[i]);
		unsigned c2 = DecodeChar(hex[i + 1]);
		if (c1 == -1 || c2 == -1)
			return Error::Fmt("Invalid hex pair '%c%c'", hex[i], hex[i + 1]);
		*out++ = (c1 << 4) | c2;
	}
	return Error();
}

BMHPAL_API std::string Encode(const void* _buf, size_t bufSize) {
	auto        buf = (const uint8_t*) _buf;
	std::string s;
	s.resize(bufSize * 2);
	size_t j = 0;
	for (size_t i = 0; i < bufSize; i++) {
		s[j]     = EncodeTable[buf[i] >> 4];
		s[j + 1] = EncodeTable[buf[i] & 0xf];
		j += 2;
	}
	return s;
}

} // namespace hex
} // namespace bmhpal