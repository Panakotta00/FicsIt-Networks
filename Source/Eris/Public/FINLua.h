#pragma once

#if PLATFORM_UNIX
#include "UnixPlatformCompilerPreSetup.h"
#elif PLATFORM_WINDOWS
#include "MSVCPlatformCompilerPreSetup.h"
#endif

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
#pragma warning(disable:4701)
#pragma warning(disable:4777)
#pragma warning(disable:4456)
#endif

//extern "C" {
#include "luaconf.h"

#undef LUA_API
#define LUA_API ERIS_API

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "eris.h"
//}

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
