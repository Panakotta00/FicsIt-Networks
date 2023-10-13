#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/WindowsHWrapper.h"

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

extern "C" {
	#include "../../../ThirdParty/eris/src/lua.h"
	#include "../../../ThirdParty/eris/src/lualib.h"
	#include "../../../ThirdParty/eris/src/lauxlib.h"
	#include "../../../ThirdParty/eris/src/eris.h"
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#pragma pop_macro("check")

THIRD_PARTY_INCLUDES_END
PRAGMA_POP_PLATFORM_DEFAULT_PACKING

#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
