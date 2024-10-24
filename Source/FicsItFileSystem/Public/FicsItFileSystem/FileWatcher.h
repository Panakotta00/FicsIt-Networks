#pragma once

#if PLATFORM_UNIX
#include "UnixFileWatcher.h"
#else
#include "WindowsFileWatcher.h"
#endif
