#include "FINStaticReflectionSourceHooks.h"

FCriticalSection UFINFactoryConnectorHook::MutexFactoryGrab;
TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> UFINFactoryConnectorHook::FactoryGrabsRunning;
