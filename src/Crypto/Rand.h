#pragma once

namespace bmhpal {
namespace crypto {

// Generate nchars of random characters, from an ascii dictionary of 62 possible characters. Each character is therefore a little short of 6 bits of entropy.
BMHPAL_API std::string RandAscii(size_t nchars);

} // namespace crypto
} // namespace bmhpal