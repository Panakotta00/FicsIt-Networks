#pragma once

#include "CoreMinimal.h"
#include "FIRTrace.h"
#include "FIRInstancedStruct.h"
#include "FINLuaProcessorStateStorage.generated.h"

/**
 * This struct holds information from the lua processor that is only needed in serialization like persistence data.
 */
USTRUCT()
struct FICSITNETWORKSLUA_API FFINLuaProcessorStateStorage {
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
	
    // Begin Struct
	bool Serialize(FStructuredArchive::FSlot Slot);
	// End Struct
			
	int32 Add(const FFIRTrace& Trace);

	int32 Add(UObject* Ref);

	int32 Add(TSharedPtr<FFIRInstancedStruct> Struct);

	FFIRTrace GetTrace(int32 id);

	UObject* GetRef(int32 id);

	TSharedPtr<FFIRInstancedStruct> GetStruct(int32 id);

	void Clear();
};

template<>
struct TStructOpsTypeTraits<FFINLuaProcessorStateStorage> : TStructOpsTypeTraitsBase2<FFINLuaProcessorStateStorage> {
	enum {
		WithStructuredSerializer = true,
    };
};