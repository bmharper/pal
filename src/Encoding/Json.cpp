#include "pch.h"
#include "Json.h"
#include "../OS/OS.h"

using namespace std;

namespace bmhpal {

namespace jsonutil {
BMHPAL_API Error LoadFile(const std::string& filename, nlohmann::json& j) {
	string raw;
	auto   err = os::ReadFile(filename, raw);
	if (!err.OK())
		return err;
	return Decode(raw, j);
}

BMHPAL_API Error SaveFile(const std::string& filename, const nlohmann::json& j) {
	return os::WriteFile(filename, j.dump(1, '\t'));
}

BMHPAL_API Error Decode(const std::string& raw, nlohmann::json& j) {
	try {
		j = nlohmann::json::parse(raw);
	} catch (std::exception& e) {
		return Error::Fmt("Error decoding json: %v", e.what());
	}
	return Error();
}

BMHPAL_API Error Decode(const void* raw, size_t rawLen, nlohmann::json& j) {
	try {
		j = nlohmann::json::parse((const char*) raw, (const char*) raw + rawLen);
	} catch (std::exception& e) {
		return Error::Fmt("Error decoding json: %v", e.what());
	}
	return Error();
}
} // namespace jsonutil

namespace jsonser {

BMHPAL_API std::string GetStr(const nlohmann::json& j, const char* key, const std::string& _default) {
	const auto& pos = j.find(key);
	if (pos != j.end() && pos->is_string())
		return pos->get<string>();
	else
		return _default;
}

BMHPAL_API int64_t GetInt64(const nlohmann::json& j, const char* key, const int64_t _default) {
	const auto& pos = j.find(key);
	if (pos != j.end() && pos->is_number())
		return pos->get<int64_t>();
	else
		return _default;
}

BMHPAL_API int32_t GetInt32(const nlohmann::json& j, const char* key, const int32_t _default) {
	const auto& pos = j.find(key);
	if (pos != j.end() && pos->is_number())
		return pos->get<int32_t>();
	else
		return _default;
}

BMHPAL_API bool GetBool(const nlohmann::json& j, const char* key, const bool _default) {
	const auto& pos = j.find(key);
	if (pos != j.end() && pos->is_boolean())
		return pos->get<bool>();
	else
		return _default;
}

BMHPAL_API time::Time GetUnixTime(const nlohmann::json& j, const char* key, const time::Time _default) {
	const auto& pos = j.find(key);
	if (pos != j.end() && pos->is_number())
		return time::Time::FromUnix(pos->get<double>());
	else
		return _default;
}

} // namespace jsonser
} // namespace bmhpal
