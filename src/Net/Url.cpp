#include "pch.h"
#include "Url.h"
#include "../Text/StringUtils.h"

namespace bmhpal {
namespace net {
namespace url {

std::string RemoveTrailingSlash(const std::string& url) {
	if (strings::EndsWith(url, "/"))
		return url.substr(0, url.length() - 1);
	else
		return url;
}

} // namespace url
} // namespace net
} // namespace bmhpal