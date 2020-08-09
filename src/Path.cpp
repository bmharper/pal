#include "pch.h"
#include "Path.h"

using namespace std;

namespace bmhpal {
namespace path {

#ifdef _WIN32
static const char PlatformSep = '\\';
#else
static const char PlatformSep = '/';
#endif

static bool IsAnySep(char c) {
	return c == '/' || c == '\\';
}

// Return the position of the last separator, or -1 if no separators
static size_t FindLastAnySep(const std::string& p) {
	const char* s = p.c_str();
	size_t      i = p.size() - 1;
	for (; i != -1; i--) {
		if (IsAnySep(s[i]))
			break;
	}
	return i;
}

BMHPAL_API std::string Dir(const std::string& p) {
	size_t sep = FindLastAnySep(p);
	if (sep == -1)
		return "";
	return p.substr(0, sep);
}

BMHPAL_API std::string Dir(const std::string& p, size_t n) {
	std::string d = p;
	for (size_t i = 0; i < n; i++)
		d = Dir(d);
	return d;
}

BMHPAL_API std::string Filename(const std::string& p) {
	size_t sep = FindLastAnySep(p);
	if (sep == -1)
		return p;
	return p.substr(sep + 1);
}

BMHPAL_API std::string Join(size_t n, const std::string** p) {
	if (n == 0)
		return "";
	string j = *p[0];
	for (size_t i = 1; i < n; i++) {
		const string& next = *p[i];
		if (next.size() == 0 || (next.size() == 1 && IsAnySep(next[0])))
			continue;
		bool sep1 = IsAnySep(j[j.size() - 1]);
		bool sep2 = IsAnySep(next[0]);
		if (sep1 && sep2) {
			j += next.substr(1);
		} else if (!sep1 && !sep2 && j.size() != 0) {
			j += PlatformSep;
			j += next;
		} else {
			j += next;
		}
	}
	return j;
}

BMHPAL_API std::string Join(const std::string& a, const std::string& b) {
	const string* v[] = {&a, &b};
	return Join(2, v);
}

BMHPAL_API std::string Join(const std::string& a, const std::string& b, const std::string& c) {
	const string* v[] = {&a, &b, &c};
	return Join(3, v);
}

BMHPAL_API std::string Join(const std::string& a, const std::string& b, const std::string& c, const std::string& d) {
	const string* v[] = {&a, &b, &c, &d};
	return Join(4, v);
}

BMHPAL_API std::string Extension(const std::string& p) {
	auto pos = p.find_last_of('.');
	if (pos == -1)
		return "";
	return p.substr(pos);
}

BMHPAL_API std::string ChangeExtension(const std::string& p, const std::string& newExt) {
	auto ext = Extension(p);
	return p.substr(0, p.size() - ext.size()) + newExt;
}

BMHPAL_API std::string SlashToNative(const std::string& p) {
	std::string r;
	r.reserve(p.size());
	for (auto cp : utfz::cp(p)) {
		if (cp == '/' || cp == '\\')
			r += PlatformSep;
		else
			utfz::encode(r, cp);
	}
	return r;
}

} // namespace path
} // namespace bmhpal