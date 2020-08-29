#pragma once

namespace bmhpal {
namespace net {
namespace url {

// Encodes a container of pair<string,string> into a=b&c=d&...
template <typename T>
std::string EncodeQueryT(const T& t) {
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

// Encodes a container of pair<string,string> into a=b&c=d&...
inline std::string EncodeQuery(const std::vector<std::pair<std::string, std::string>>& t) {
	return EncodeQueryT(t);
}

// Remove trailing slash from a url
std::string RemoveTrailingSlash(const std::string& url);

} // namespace url
} // namespace net
} // namespace bmhpal