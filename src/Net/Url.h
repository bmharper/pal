#pragma once

namespace bmhpal {
namespace net {
namespace url {

// Encodes a container of pair<string,string> into a=b&c=d&...
template <typename T>
std::string EncodeQuery(const T& t) {
	std::string r;
	for (const auto& p : t) {
		r += modp::url_encode(p.first);
		r += "=";
		r += modp::url_encode(p.second);
		r += "&";
	}
	if (t.size() != 0) {
		r.erase(r.end() - 1);
	}
	return r;
}

} // namespace url
} // namespace net
} // namespace bmhpal