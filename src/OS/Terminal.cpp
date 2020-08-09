#include "pch.h"
#include "Terminal.h"

namespace bmhpal {
namespace os {
namespace termcolor {

BMHPAL_API const char* Normal  = "\x1B[0m";
BMHPAL_API const char* Red     = "\x1B[31m";
BMHPAL_API const char* Green   = "\x1B[32m";
BMHPAL_API const char* Yellow  = "\x1B[33m";
BMHPAL_API const char* Blue    = "\x1B[34m";
BMHPAL_API const char* Magenta = "\x1B[35m";
BMHPAL_API const char* Cyan    = "\x1B[36m";
BMHPAL_API const char* White   = "\x1B[37m";

} // namespace termcolor
} // namespace os
} // namespace bmhpal