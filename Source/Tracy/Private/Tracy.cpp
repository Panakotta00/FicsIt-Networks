#if PLATFORM_UNIX
#include "UnixPlatformCompilerPreSetup.h"
#elif PLATFORM_WINDOWS
#include "MSVCPlatformCompilerPreSetup.h"
#endif

THIRD_PARTY_INCLUDES_START

#ifdef TRACY_ENABLE
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/WindowsHWrapper.h"

#include "TracyClient.cpp"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma warning(pop)
#endif

#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"

#endif

THIRD_PARTY_INCLUDES_END
