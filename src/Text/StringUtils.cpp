#include "pch.h"

namespace bmhpal {

// This is from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
template <typename TINT, typename TCH>
size_t ItoAT(TINT value, TCH* result, int base) {
	if (base < 2 || base > 36) {
		*result = '\0';
		return 0;
	}

	TCH *ptr = result, *ptr1 = result, tmp_char;
	TINT tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
	} while (value);

	// Apply negative sign
	if (tmp_value < 0)
		*ptr++ = '-';
	size_t written = (size_t)(ptr - result);
	*ptr--         = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--   = *ptr1;
		*ptr1++  = tmp_char;
	}
	return written;
}

BMHPAL_API size_t ItoA(int value, char* result, int base) {
	return ItoAT(value, result, base);
}

BMHPAL_API size_t I64toA(int64_t value, char* result, int base) {
	return ItoAT(value, result, base);
}

BMHPAL_API std::string ItoA(int value, int base) {
	char buf[34];
	ItoA(value, buf, base);
	return buf;
}

BMHPAL_API std::string I64toA(int64_t value, int base) {
	char buf[66];
	I64toA(value, buf, base);
	return buf;
}

namespace strings {

// Ruby: (0..127).each { |c| low = c >= 'A'.ord && c <= 'Z'.ord ? c + 'a'.ord - 'A'.ord : c;  print("#{low},") }
const char ToLower128[128] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127};

inline char ToLowerChar(char c) {
	if ((unsigned char) c < (unsigned char) 128)
		return ToLower128[c];
	else
		return c;
}

BMHPAL_API bool EqualsNoCase(const char* a, const char* b) {
	size_t i = 0;
	for (; a[i] && b[i]; i++) {
		if (ToLowerChar(a[i]) != ToLowerChar(b[i]))
			return false;
	}
	// both a[i] and b[i] must be zero, otherwise string length differs
	return a[i] == b[i];
}

BMHPAL_API std::string Join(const std::vector<std::string>& parts, const char* joiner) {
	std::string r;
	for (size_t i = 0; i < parts.size(); i++) {
		r += parts[i];
		if (i != parts.size() - 1)
			r += joiner;
	}
	return r;
}

template <bool CaseSensitive>
bool MatchWildcardT(const char* s, const char* p) {
	if (*p == '*') {
		while (*p == '*')
			++p;
		if (*p == 0)
			return true;
		while (*s != 0 && !MatchWildcardT<CaseSensitive>(s, p)) {
			int cp;
			if (!utfz::next(s, cp))
				break;
		}
		return *s != 0;
	} else if (*p == 0 || *s == 0) {
		return *p == *s;
	} else {
		int px = utfz::decode(p);
		int sx = utfz::decode(s);

		if ((CaseSensitive ? (px == sx) : ::tolower(px) == ::tolower(sx)) || *p == '?') {
			int cp;
			utfz::next(s, cp);
			utfz::next(p, cp);
			return MatchWildcardT<CaseSensitive>(s, p);
		} else {
			return false;
		}
	}
}

BMHPAL_API bool MatchWildcard(const std::string& s, const std::string& p) {
	return MatchWildcardT<true>(s.c_str(), p.c_str());
}
BMHPAL_API bool MatchWildcard(const char* s, const char* p) {
	return MatchWildcardT<true>(s, p);
}
BMHPAL_API bool MatchWildcardNoCase(const std::string& s, const std::string& p) {
	return MatchWildcardT<false>(s.c_str(), p.c_str());
}
BMHPAL_API bool MatchWildcardNoCase(const char* s, const char* p) {
	return MatchWildcardT<false>(s, p);
}

BMHPAL_API std::string FormatBytes(int64_t bytes) {
	const int64_t MB = 1024 * 1024;
	const int64_t GB = 1024 * 1024 * 1024;
	const int64_t TB = 1024 * 1024 * 1024 * (int64_t) 1024;
	const int64_t PB = 1024 * 1024 * 1024 * (int64_t) 1024 * (int64_t) 1024;
	if (bytes < 1024)
		return tsf::fmt("%v bytes", bytes);
	else if (bytes < 1024 * 1024)
		return tsf::fmt("%.0f KB", bytes / (double) 1024);
	else if (bytes < GB)
		return tsf::fmt("%.1f MB", bytes / (double) MB);
	else if (bytes < TB)
		return tsf::fmt("%.2f GB", bytes / (double) GB);
	else if (bytes < PB)
		return tsf::fmt("%.3f TB", bytes / (double) TB);
	else
		return tsf::fmt("%.4f PB", bytes / (double) PB);
}

} // namespace strings
} // namespace bmhpal