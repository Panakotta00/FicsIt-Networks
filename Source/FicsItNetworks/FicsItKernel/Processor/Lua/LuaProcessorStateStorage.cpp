#include "LuaProcessorStateStorage.h"


#include "FicsItNetworks/FicsItNetworksModule.h"
#include "FicsItNetworks/Network/FINDynamicStructHolder.h"

bool FFINLuaProcessorStateStorage::Serialize(FStructuredArchive::FSlot Slot) {
	if (!Slot.GetUnderlyingArchive().IsSaveGame()) return false;
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	Record.EnterField(SA_FIELD_NAME(TEXT("Traces"))).GetUnderlyingArchive() << Traces;
	Record.EnterField(SA_FIELD_NAME(TEXT("References"))) << References;
	Record.EnterField(SA_FIELD_NAME(TEXT("Thread"))) << Thread;
	Record.EnterField(SA_FIELD_NAME(TEXT("Globals"))) << Globals;

	FStructuredArchive::FSlot Ar = Record.EnterField(SA_FIELD_NAME(TEXT("Structs")));
	
	int StructNum = Structs.Num();
	Ar << StructNum;
	if (Record.GetUnderlyingArchive().IsLoading()) Structs.Empty();
	for (int i = 0; i < StructNum; ++i) {
		int j = i;
		if (Record.GetUnderlyingArchive().IsLoading()) j = Structs.Add(MakeShared<FFINDynamicStructHolder>());
		TSharedPtr<FFINDynamicStructHolder> holder = Structs[j];
		if (holder.IsValid() && holder.Get()) Ar.GetUnderlyingArchive() << *holder.Get();
		else {
			FFINDynamicStructHolder h;
			Ar.GetUnderlyingArchive() << h;
		}
	}
	for (UObject* r : References) {
		if (r && r->GetClass()->IsChildOf(UClass::StaticClass())) r->AddToRoot();
	}
	return true;
}


int32 FFINLuaProcessorStateStorage::Add(const FFINNetworkTrace& Trace) {
	return Traces.AddUnique(Trace);
}

int32 FFINLuaProcessorStateStorage::Add(UObject* Ref) {
	return References.AddUnique(Ref);
}

int32 FFINLuaProcessorStateStorage::Add(TSharedPtr<FFINDynamicStructHolder> Struct) {
	return Structs.Add(Struct);
}

FFINNetworkTrace FFINLuaProcessorStateStorage::GetTrace(int32 id) {
	return Traces[id];
}

UObject* FFINLuaProcessorStateStorage::GetRef(int32 id) {
	return References[id];
}

TSharedPtr<FFINDynamicStructHolder> FFINLuaProcessorStateStorage::GetStruct(int32 id) {
	if (id >= Structs.Num()) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Unable to find struct in lua processor state storage with id %i"), id);
		return MakeShared<FFINDynamicStructHolder>();
	}
	return Structs[id];
}

void FFINLuaProcessorStateStorage::Clear() {
	Traces.Empty();
	References.Empty();
	Structs.Empty();
	Thread.Empty();
	Globals.Empty();
}
