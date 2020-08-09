#pragma once

#include "../Error/Error.h"
#include "../Time/Time_.h"

namespace bmhpal {

namespace jsonutil {
BMHPAL_API Error LoadFile(const std::string& filename, nlohmann::json& j);
BMHPAL_API Error SaveFile(const std::string& filename, const nlohmann::json& j);
BMHPAL_API Error Decode(const std::string& raw, nlohmann::json& j);
BMHPAL_API Error Decode(const void* raw, size_t rawLen, nlohmann::json& j);
} // namespace jsonutil

// json serialization
namespace jsonser {
BMHPAL_API std::string GetStr(const nlohmann::json& j, const char* key, const std::string& _default = "");
BMHPAL_API int64_t     GetInt64(const nlohmann::json& j, const char* key, const int64_t _default = 0);
BMHPAL_API int32_t     GetInt32(const nlohmann::json& j, const char* key, const int32_t _default = 0);
BMHPAL_API bool        GetBool(const nlohmann::json& j, const char* key, const bool _default = false);
BMHPAL_API time::Time GetUnixTime(const nlohmann::json& j, const char* key, const time::Time _default = time::Time());

inline bool Has(const nlohmann::json& j, const char* key) {
	return j.find(key) != j.end();
}
} // namespace jsonser

} // namespace bmhpal
