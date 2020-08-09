#pragma once
// XLib.h defines a bunch of unsavoury macros such as Status, Bool, None.
// Here we undef those after including all the X libraries that we need

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysym.h>

// Remove ridiculous X11 defines
#ifdef None
#undef None
#endif
#ifdef Status
#undef Status
#endif
#ifdef Success
#undef Success
#endif
#ifdef Bool
#undef Bool
#endif
#ifdef False
#undef False
#endif
#ifdef True
#undef True
#endif

namespace X11 {
enum {
	None    = 0,
	Success = 0,
	True    = 1,
	False   = 0,
};
} // namespace X11

// Unfortunately these need to be in the global namespace in order for OpenGL loading code to compile.
// But at least having them as typedefs is better than macros
typedef int Bool;
typedef int Status;
