#pragma once

#define TT_MODULE_NAME pal
#define TESTFUNC(name) TT_TEST_FUNC(NULL, NULL, TTSizeSmall, name, TTParallelDontCare)

#include <src/pal.h>

#include <third_party/TinyTest/TinyTest.h>
