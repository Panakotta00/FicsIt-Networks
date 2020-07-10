#include "LuaProcessorStateStorage.h"

FDynamicStructHolder::FDynamicStructHolder() {}

FDynamicStructHolder::FDynamicStructHolder(UStruct* Struct) : Struct(Struct) {
	Data = FMemory::Malloc(Struct->GetStructureSize());
	Struct->InitializeStruct(Data);
}

FDynamicStructHolder::FDynamicStructHolder(UStruct* Struct, void* Data) : Data(Data), Struct(Struct) {}

FDynamicStructHolder::~FDynamicStructHolder() {
	if (Data) {
		Struct->DestroyStruct(Data);
		FMemory::Free(Data);
		Data = nullptr;
	}
}

bool FDynamicStructHolder::Serialize(FArchive& Ar) {
	if (Ar.IsLoading() && Data && Struct) {
		Struct->DestroyStruct(Data);
		FMemory::Free(Data);
		Data = nullptr;
	}
	Ar << Struct;
	if (Ar.IsLoading() && Struct) {
		Data = FMemory::Malloc(Struct->GetStructureSize());
		Struct->InitializeStruct(Data);
	}
	if (Data && Struct) Struct->SerializeBin(Ar, Data);
	return true;
}

UStruct* FDynamicStructHolder::GetStruct() {
	return Struct;
}

void* FDynamicStructHolder::GetData() {
	return Data;
}

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
		if (Ar.IsLoading()) j = Structs.Add(MakeShared<FDynamicStructHolder>());
		TSharedPtr<FDynamicStructHolder> holder = Structs[j];
		if (holder.IsValid() && holder.Get()) Ar << *holder.Get();
		else {
			FDynamicStructHolder h;
			Ar << h;
		}
	}
	for (UObject* r : References) {
		if (r->GetClass()->IsChildOf(UClass::StaticClass())) r->AddToRoot();
	}
}

int32 ULuaProcessorStateStorage::Add(const FFINNetworkTrace& Trace) {
	return Traces.AddUnique(Trace);
}

int32 ULuaProcessorStateStorage::Add(UObject* Ref) {
	return References.AddUnique(Ref);
}

int32 ULuaProcessorStateStorage::Add(TSharedPtr<FDynamicStructHolder> Struct) {
	return Structs.Add(Struct);
}

FFINNetworkTrace ULuaProcessorStateStorage::GetTrace(int32 id) {
	return Traces[id];
}

UObject* ULuaProcessorStateStorage::GetRef(int32 id) {
	return References[id];
}

TSharedPtr<FDynamicStructHolder> ULuaProcessorStateStorage::GetStruct(int32 id) {
	return Structs[id];
}
