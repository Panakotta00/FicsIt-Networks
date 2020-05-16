#include "LuaProcessorStateStorage.h"

void ULuaProcessorStateStorage::Serialize(FArchive& Ar) {
	Ar << Thread;
	Ar << Globals;
	Ar << Traces;
	Ar << References;
}

int32 ULuaProcessorStateStorage::Add(const FFINNetworkTrace& Trace) {
	return Traces.AddUnique(Trace);
}

int32 ULuaProcessorStateStorage::Add(UObject* Ref) {
	return References.AddUnique(Ref);
}

FFINNetworkTrace ULuaProcessorStateStorage::GetTrace(int32 id) {
	return Traces[id];
}

UObject* ULuaProcessorStateStorage::GetRef(int32 id) {
	return References[id];
}