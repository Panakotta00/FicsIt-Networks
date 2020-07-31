#include "LuaProcessorStateStorage.h"

#include "Network/FINDynamicStructHolder.h"

void ULuaProcessorStateStorage::Serialize(FArchive& Ar) {
	Ar << Thread;
	Ar << Globals;
	Ar << Traces;
	Ar << References;
	Ar << PullState;
	Ar << Timeout;
	Ar << PullStart;
	int StructNum = Structs.Num();
	Ar << StructNum;
	if (Ar.IsLoading()) Structs.Empty();
	for (int i = 0; i < StructNum; ++i) {
		int j = i;
		if (Ar.IsLoading()) j = Structs.Add(MakeShared<FFINDynamicStructHolder>());
		TSharedPtr<FFINDynamicStructHolder> holder = Structs[j];
		if (holder.IsValid() && holder.Get()) Ar << *holder.Get();
		else {
			FFINDynamicStructHolder h;
			Ar << h;
		}
	}
	for (UObject* r : References) {
		if (r && r->GetClass()->IsChildOf(UClass::StaticClass())) r->AddToRoot();
	}
}

int32 ULuaProcessorStateStorage::Add(const FFINNetworkTrace& Trace) {
	return Traces.AddUnique(Trace);
}

int32 ULuaProcessorStateStorage::Add(UObject* Ref) {
	return References.AddUnique(Ref);
}

int32 ULuaProcessorStateStorage::Add(TSharedPtr<FFINDynamicStructHolder> Struct) {
	return Structs.Add(Struct);
}

FFINNetworkTrace ULuaProcessorStateStorage::GetTrace(int32 id) {
	return Traces[id];
}

UObject* ULuaProcessorStateStorage::GetRef(int32 id) {
	return References[id];
}

TSharedPtr<FFINDynamicStructHolder> ULuaProcessorStateStorage::GetStruct(int32 id) {
	return Structs[id];
}
