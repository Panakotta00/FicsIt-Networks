#include "FINLuaRuntimePersistence.h"

#include "FicsItNetworksLuaModule.h"
#include "FINUtils.h"
#include "Engine/World.h"
#include "Util/SemVersion.h"

bool FFINLuaRuntimePersistenceState::Serialize(FStructuredArchive::FSlot Slot) {
	if (!Slot.GetUnderlyingArchive().IsSaveGame()) return false;
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	Record.EnterField(SA_FIELD_NAME(TEXT("Traces"))).GetUnderlyingArchive() << Traces;
	Record.EnterField(SA_FIELD_NAME(TEXT("References"))) << References;
	Record.EnterField(SA_FIELD_NAME(TEXT("Thread"))) << LuaData;
	FString Str;
	Record.EnterField(SA_FIELD_NAME(TEXT("Globals"))) << Str;

	FVersion version = UFINUtils::GetFINSaveVersion(GWorld);
	if (FVersion(0, 3, 19).Compare(version) == 1) return false;

	int32 StructNum = Structs.Num();
	FStructuredArchiveArray Array = Record.EnterArray(SA_FIELD_NAME(TEXT("Structs")), StructNum);

	if (Record.GetUnderlyingArchive().IsLoading()) Structs.Empty();
	for (int i = 0; i < StructNum; ++i) {
		if (Record.GetUnderlyingArchive().IsLoading()) Structs.Add(MakeShared<FFIRInstancedStruct>());
		TSharedPtr<FFIRInstancedStruct> holder = Structs[i];
		if (holder) {
			holder->Serialize(Array.EnterElement());
		} else {
			FFIRInstancedStruct().Serialize(Array.EnterElement());
		}
	}

	Record.EnterField(SA_FIELD_NAME(TEXT("Failure"))) << Failure;

	return true;
}

int32 FFINLuaRuntimePersistenceState::Add(const FFIRTrace& Trace) {
	return Traces.AddUnique(Trace);
}

int32 FFINLuaRuntimePersistenceState::Add(UObject* Ref) {
	return References.AddUnique(Ref);
}

int32 FFINLuaRuntimePersistenceState::Add(TSharedPtr<FFIRInstancedStruct> Struct) {
	return Structs.Add(Struct);
}

FFIRTrace FFINLuaRuntimePersistenceState::GetTrace(int32 id) {
	return Traces[id];
}

UObject* FFINLuaRuntimePersistenceState::GetRef(int32 id) {
	return References[id];
}

TSharedPtr<FFIRInstancedStruct> FFINLuaRuntimePersistenceState::GetStruct(int32 id) {
	if (id >= Structs.Num()) {
		UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Unable to find struct in lua processor state storage with id %i"), id);
		return MakeShared<FFIRInstancedStruct>();
	}
	return Structs[id];
}

void FFINLuaRuntimePersistenceState::Clear() {
	Failure.Empty();
	Traces.Empty();
	References.Empty();
	Structs.Empty();
	LuaData.Empty();
}
