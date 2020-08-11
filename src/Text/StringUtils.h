#pragma once

namespace bmhpal {

inline int64_t atoi64(const char* s) {
#ifdef _MSC_VER
	return _atoi64(s);
#else
	return (int64_t) atoll(s);
#endif
}

BMHPAL_API size_t ItoA(int value, char* result, int base = 10);
BMHPAL_API size_t I64toA(int64_t value, char* result, int base = 10);
BMHPAL_API std::string ItoA(int value, int base = 10);
BMHPAL_API std::string I64toA(int64_t value, int base = 10);

namespace strings {

template <typename T>
std::vector<std::basic_string<T>> Split(const std::basic_string<T>& s, char split) {
	std::vector<std::basic_string<T>> res;
	const size_t                      len = s.size();
	size_t                            pos = 0;
	for (size_t i = 0; i < len; i++) {
		if (s[i] == split) {
			res.emplace_back(s.substr(pos, i - pos));
			pos = i + 1;
		}
	}
	if (len != pos)
		res.emplace_back(s.substr(pos, len - pos));
	return res;
}

template <typename T>
std::basic_string<T> Replace(const std::basic_string<T>& s, const std::basic_string<T>& find, const std::basic_string<T>& rep, size_t maxn = -1) {
	std::basic_string<T> snew;
	size_t               n   = 0;
	size_t               pos = 0;
	for (; pos < s.size() && n < maxn; n++) {
		size_t findpos = s.find(find, pos);
		if (findpos == -1)
			break;
		snew.append(s.c_str() + pos, findpos - pos);
		snew.append(rep);
		pos = findpos + find.size();
	}
	snew.append(s.c_str() + pos, s.size() - pos);
	return snew;
}

template <typename T>
std::basic_string<T> toupper(const std::basic_string<T>& s) {
	auto                 len = s.size();
	std::basic_string<T> r;
	r.reserve(len);
	for (size_t i = 0; i < len; i++) {
		if (s[i] >= 'a' && s[i] <= 'z')
			r += s[i] - ('a' - 'A');
		else
			r += s[i];
	}
	return r;
}

template <typename T>
std::basic_string<T> tolower(const std::basic_string<T>& s) {
	auto                 len = s.size();
	std::basic_string<T> r;
	r.reserve(len);
	for (size_t i = 0; i < len; i++) {
		if (s[i] >= 'A' && s[i] <= 'Z')
			r += s[i] + ('a' - 'A');
		else
			r += s[i];
	}
	return r;
}

inline bool IsWhite(char c) {
	return c == '\t' || c == ' ' || c == '\r' || c == '\n';
}

// Trims tab,space,newline,CR from left and right sides of string
template <typename T>
std::basic_string<T> TrimSpace(const std::basic_string<T>& s) {
	std::basic_string<T> r;
	auto                 len     = s.size();
	bool                 atStart = true;
	for (size_t i = 0; i < len; i++) {
		char c = s[i];
		if (atStart) {
			if (!IsWhite(c)) {
				atStart = false;
				r += c;
			}
		} else {
			r += c;
		}
	}
	size_t j = r.size() - 1;
	for (; j != -1; j--) {
		if (!IsWhite(r[j]))
			break;
	}
	if (j != r.size() - 1)
		r.erase(r.begin() + j + 1, r.end());
	return r;
}

template <typename T>
bool EqualsNoCase(const std::basic_string<T>& a, const std::basic_string<T>& b) {
	if (a.size() != b.size())
		return false;
	size_t len = a.size();
	for (size_t i = 0; i < len; i++) {
		int _a = a[i];
		int _b = b[i];
		_a     = (_a >= 'A' && _a <= 'Z') ? _a + 'a' - 'A' : _a;
		_b     = (_b >= 'A' && _b <= 'Z') ? _b + 'a' - 'A' : _b;
		if (_a != _b)
			return false;
	}
	return true;
}

BMHPAL_API bool EqualsNoCase(const char* a, const char* b);

template <typename T>
bool StartsWith(const std::basic_string<T>& a, const std::basic_string<T>& b) {
	return a.find(b) == 0;
}

inline bool StartsWith(const std::string& a, const char* b) {
	return a.find(b) == 0;
}

template <typename T>
bool EndsWith(const std::basic_string<T>& a, const std::basic_string<T>& b) {
	return a.rfind(b) == a.size() - b.size();
}

inline bool EndsWith(const std::string& a, const char* b) {
	return a.rfind(b) == a.size() - strlen(b);
}

BMHPAL_API std::string Join(const std::vector<std::string>& parts, const char* joiner);

BMHPAL_API bool MatchWildcard(const std::string& s, const std::string& p);
BMHPAL_API bool MatchWildcard(const char* s, const char* p);
BMHPAL_API bool MatchWildcardNoCase(const std::string& s, const std::string& p);
BMHPAL_API bool MatchWildcardNoCase(const char* s, const char* p);

// Returns one of
// 5 bytes
// 3 KB
// 5.1 MB
// 7.12 GB
// 9.345 TB
// 11.5678 PB
BMHPAL_API std::string FormatBytes(int64_t bytes);

} // namespace strings
} // namespace bmhpal