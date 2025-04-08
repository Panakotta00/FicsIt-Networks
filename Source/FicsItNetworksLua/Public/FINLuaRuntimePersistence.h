#pragma once

#include "CoreMinimal.h"
#include "FIRTrace.h"
#include "FIRInstancedStruct.h"
#include "FINLuaRuntimePersistence.generated.h"

/**
 * This struct holds information from the lua processor that is only needed in serialization like persistence data.
 */
USTRUCT()
struct FICSITNETWORKSLUA_API FFINLuaRuntimePersistenceState {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TArray<FFIRTrace> Traces;

	UPROPERTY(SaveGame)
	TArray<UObject*> References;

	TArray<TSharedPtr<FFIRInstancedStruct>> Structs;

public:
	UPROPERTY(SaveGame)
	FString LuaData;

	/**
	 * If saving the persistence state failed,
	 * this string has a length greater than zero.
	 * It will contain the reason saving the state failed.
	 */
	UPROPERTY(SaveGame)
	FString Failure;

	FFINLuaRuntimePersistenceState() = default;
	FFINLuaRuntimePersistenceState(const FString& Failure) : Failure(Failure) {}

    // Begin Struct
	bool Serialize(FStructuredArchive::FSlot Slot);
	// End Struct

	bool IsFailure() const {
		return !Failure.IsEmpty();
	}
			
	int32 Add(const FFIRTrace& Trace);

	int32 Add(UObject* Ref);

	int32 Add(TSharedPtr<FFIRInstancedStruct> Struct);

	FFIRTrace GetTrace(int32 id);

	UObject* GetRef(int32 id);

	TSharedPtr<FFIRInstancedStruct> GetStruct(int32 id);

	void Clear();
};

template<>
struct TStructOpsTypeTraits<FFINLuaRuntimePersistenceState> : TStructOpsTypeTraitsBase2<FFINLuaRuntimePersistenceState> {
	enum {
		WithStructuredSerializer = true,
    };
};