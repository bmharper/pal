#include "pch.h"

using namespace bmhpal;
using namespace std;

TESTFUNC(StringReplace) {
	TTASSERT(strings::Replace<char>("one two one three", "one", "FIVE", 0) == "one two one three");
	TTASSERT(strings::Replace<char>("one two one three", "one", "FIVE", 1) == "FIVE two one three");
	TTASSERT(strings::Replace<char>("one two one three", "one", "FIVE") == "FIVE two FIVE three");
	TTASSERT(strings::Replace<char>("ab", "a", "") == "b");
	TTASSERT(strings::Replace<char>("ab", "ab", "") == "");
	TTASSERT(strings::Replace<char>("ab", "b", "") == "a");
	TTASSERT(strings::Replace<char>("ab", "c", "") == "ab");
	TTASSERT(strings::Replace<char>("<abab>", "ab", "c") == "<cc>");
}

TESTFUNC(StringTrim) {
	TTASSERT(strings::TrimSpace<char>("") == "");
	TTASSERT(strings::TrimSpace<char>(" ") == "");
	TTASSERT(strings::TrimSpace<char>("  ") == "");
	TTASSERT(strings::TrimSpace<char>("a") == "a");
	TTASSERT(strings::TrimSpace<char>(" a ") == "a");
	TTASSERT(strings::TrimSpace<char>(" a b ") == "a b");
	TTASSERT(strings::TrimSpace<char>("a b  ") == "a b");
	TTASSERT(strings::TrimSpace<char>("  a b") == "a b");
}

TESTFUNC(EqualsNoCase) {
	TTASSERT(strings::EqualsNoCase("", ""));
	TTASSERT(strings::EqualsNoCase("a", "A"));
	TTASSERT(strings::EqualsNoCase("ab", "Ab"));
	TTASSERT(!strings::EqualsNoCase("ab", "abc"));
	TTASSERT(!strings::EqualsNoCase("abc", "ab"));
	TTASSERT(!strings::EqualsNoCase("", "a"));
	TTASSERT(!strings::EqualsNoCase("a", ""));
}

TESTFUNC(StringDiff) {
	auto check = [](string a, string b) {
		auto dist = diff::StringDistance(a, b);
		tsf::print("%20v -> %20v, %v\n", a, b, dist);
	};
	check("", "");
	check("", "a");
	check("a", "");
	check("Link", "Link");
	check("Link", "JourneyApps");
	check("foobar123junk", "foobarjunk");
	check("foobar", "foobarJUICE");
	check("like", "l1ke");
}

TESTFUNC(StringPredicates) {
	TTASSERT(strings::StartsWith<char>("a", "a"));
	TTASSERT(strings::StartsWith<char>("aA", "a"));
	TTASSERT(!strings::StartsWith<char>("aA", "A"));
	TTASSERT(strings::EndsWith<char>("a", "a"));
	TTASSERT(strings::EndsWith<char>("aA", "A"));
	TTASSERT(strings::EndsWith<char>("bdaaabd", "bd"));
	TTASSERT(!strings::EndsWith<char>("bdaaa", "bd"));
}

TESTFUNC(ByteFormat) {
	TTASSEQ(strings::FormatBytes(0), "0 bytes");
	TTASSEQ(strings::FormatBytes(1), "1 bytes");
	TTASSEQ(strings::FormatBytes(1023), "1023 bytes");
	TTASSEQ(strings::FormatBytes(1024), "1 KB");
	TTASSEQ(strings::FormatBytes(1300), "1 KB");
	TTASSEQ(strings::FormatBytes(1600), "2 KB");
	TTASSEQ(strings::FormatBytes(2047), "2 KB");
	TTASSEQ(strings::FormatBytes(3300), "3 KB");
	TTASSEQ(strings::FormatBytes(3600), "4 KB");
	TTASSEQ(strings::FormatBytes(1024 * 1024), "1.0 MB");
	TTASSEQ(strings::FormatBytes(1324 * 1024), "1.3 MB");
	TTASSEQ(strings::FormatBytes(1024 * 1024 * 1024), "1.00 GB");
	TTASSEQ(strings::FormatBytes(1324 * 1024 * 1024), "1.29 GB");
	TTASSEQ(strings::FormatBytes(1024 * 1024 * (int64_t) 1024 * (int64_t) 1024), "1.000 TB");
	TTASSEQ(strings::FormatBytes(1324 * 1024 * (int64_t) 1024 * (int64_t) 1024), "1.293 TB");
	TTASSEQ(strings::FormatBytes(1024 * 1024 * 1024 * (int64_t) 1024 * (int64_t) 1024), "1.0000 PB");
	TTASSEQ(strings::FormatBytes(1325 * 1024 * 1024 * (int64_t) 1024 * (int64_t) 1024), "1.2939 PB");
}
