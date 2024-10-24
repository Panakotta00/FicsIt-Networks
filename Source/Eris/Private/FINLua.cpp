#include "FINLua.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/WindowsHWrapper.h"
#endif

PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
THIRD_PARTY_INCLUDES_START

#pragma push_macro("check")
#undef check

#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning(disable:4005)
#pragma warning(disable:4310)
#pragma warning(disable:4334)
#pragma warning(disable:4374)
#pragma warning(disable:4456)
#pragma warning(disable:4701)
#pragma warning(disable:4777)
#endif

#include "eris.c"
#include "lapi.c"
#include "lauxlib.c"
#include "lbaselib.c"
#include "lcode.c"
#include "lcorolib.c"
#include "lctype.c"
#include "ldblib.c"
#include "ldebug.c"
#include "ldo.c"
#include "ldump.c"
#include "lfunc.c"
#include "lgc.c"
#include "linit.c"
#include "liolib.c"
#include "llex.c"
#include "lmathlib.c"
#include "lmem.c"
#include "loadlib.c"
#include "lobject.c"
#include "lopcodes.c"
#include "loslib.c"
#include "lparser.c"
#include "lstate.c"
#include "lstring.c"
#include "lstrlib.c"
#include "ltable.c"
#include "ltablib.c"
#include "ltm.c"
#include "lundump.c"
#include "lutf8lib.c"
#include "lvm.c"
#include "lzio.c"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#pragma pop_macro("check")

THIRD_PARTY_INCLUDES_END
PRAGMA_POP_PLATFORM_DEFAULT_PACKING

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif