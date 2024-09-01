#include "Reflection/Source/Static/FIRSourceStaticHooks.h"

FCriticalSection UFIRFactoryConnectorHook::MutexFactoryGrab;
TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> UFIRFactoryConnectorHook::FactoryGrabsRunning;
