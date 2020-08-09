#include "pch.h"

using namespace std;

namespace bmhpal {
namespace crypto {

static const int  AsciiAlphabetSize                    = 62;
static const char AsciiAlphabet[AsciiAlphabetSize + 1] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

BMHPAL_API std::string RandAscii(size_t nchars) {
	if (nchars == 0)
		return "";
	string raw;
	raw.resize(nchars);
	RAND_bytes((unsigned char*) &raw[0], (int) nchars);
	string enc;
	enc.resize(nchars);
	for (size_t i = 0; i < nchars; i++)
		enc[i] = AsciiAlphabet[(uint8_t) raw[i] % AsciiAlphabetSize];
	return enc;
}

} // namespace crypto
} // namespace bmhpal