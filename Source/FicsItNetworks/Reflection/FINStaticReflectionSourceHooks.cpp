#include "FINStaticReflectionSourceHooks.h"

TMap<UClass*, bool> UFINFunctionHook::IsRegistered;
TMap<UClass*, TSet<FWeakObjectPtr>> UFINFunctionHook::Senders;
TMap<UClass*, TSharedRef<FCriticalSection>> UFINFunctionHook::Mutex;
FCriticalSection UFINFactoryConnectorHook::MutexFactoryGrab;
TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> UFINFactoryConnectorHook::FactoryGrabsRunning;
