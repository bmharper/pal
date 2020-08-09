#pragma once

namespace bmhpal {
namespace path {

BMHPAL_API std::string Dir(const std::string& p);                                        // Return everything up to the last slash, or an empty string, if no slashes exist
BMHPAL_API std::string Dir(const std::string& p, size_t n);                              // Call Dir() recursively, n times
BMHPAL_API std::string Filename(const std::string& p);                                   // Return everything after the last slash, or the entire string, if no slashes exist
BMHPAL_API std::string Extension(const std::string& p);                                  // Return everything after the last dot, including the dot. If no dot, return an empty string
BMHPAL_API std::string ChangeExtension(const std::string& p, const std::string& newExt); // Change the extension, which is the exact string returned by 'Extension'. You must include the dot in the replacement
BMHPAL_API std::string SlashToNative(const std::string& p);                              // Change forward and backslashes to the native platform type

BMHPAL_API std::string Join(size_t n, const std::string** p);                                                        // Join n paths
BMHPAL_API std::string Join(const std::string& a, const std::string& b);                                             // Join two paths
BMHPAL_API std::string Join(const std::string& a, const std::string& b, const std::string& c);                       // Join three paths
BMHPAL_API std::string Join(const std::string& a, const std::string& b, const std::string& c, const std::string& d); // Join four paths

} // namespace path
} // namespace bmhpal