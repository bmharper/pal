#pragma once

#include "../Error/Error.h"

namespace bmhpal {
namespace hex {
BMHPAL_API unsigned DecodeChar(char hex); // Returns -1 if invalid
BMHPAL_API Error    Decode(const char* hex, void* out, size_t outBufferSize);
BMHPAL_API std::string Encode(const void* buf, size_t bufSize);
} // namespace hex
} // namespace bmhpal
