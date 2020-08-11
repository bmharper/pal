#pragma once

#ifdef _WIN32
//#define BMHPAL_API __declspec(dllimport)
#define BMHPAL_API
#else
#define BMHPAL_API
#endif

#include "common.h"

#include "PlatformDefinitions.h"
#include "Alloc.h"
#include "Algo/BinarySearch.h"
#include "Algo/CacheEviction.h"
#include "Algo/Filter.h"
#include "Containers/Queue.h"
#include "Containers/ObjQueue.h"
#include "Crypto/Rand.h"
#include "Diff/Diff.h"
#include "Encoding/Json.h"
#include "Error/Asserts.h"
#include "Error/Error.h"
#include "Error/CommonErrors.h"
#include "Error/StackTrace.h"
#include "Hash/crc32.h"
#include "Hash/Sig16.h"
#include "Hash/Sig32.h"
#include "OS/OS.h"
#include "OS/Terminal.h"
#include "Path.h"
#include "Math_.h"
#include "Net/Http.h"
#include "Net/Url.h"
#include "Time/Time_.h"
#include "Text/ConvertUTF.h"
#include "Text/StringUtils.h"
#include "Viz/Viz.h"
