#pragma once

#include "CoreMinimal.h"
#include "FicsItNetworks/Network/FINDynamicStructHolder.h"
#include "FicsItNetworks/Network/FINNetworkTrace.h"
#include "LuaProcessorStateStorage.generated.h"

/**
 * This struct holds information from the lua processor that is only needed in serialization like persistence data.
 */
USTRUCT()
struct FFINLuaProcessorStateStorage {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TArray<FFINNetworkTrace> Traces;

	UPROPERTY(SaveGame)
	TArray<UObject*> References;

	TArray<TSharedPtr<FFINDynamicStructHolder>> Structs;

public:
	UPROPERTY(SaveGame)
	FString Thread;
	
	UPROPERTY(SaveGame)
	FString Globals;
	
    // Begin Struct
	bool Serialize(FStructuredArchive::FSlot Slot);
	// End Struct
			
	int32 Add(const FFINNetworkTrace& Trace);

	int32 Add(UObject* Ref);

	int32 Add(TSharedPtr<FFINDynamicStructHolder> Struct);

	FFINNetworkTrace GetTrace(int32 id);

	UObject* GetRef(int32 id);

	TSharedPtr<FFINDynamicStructHolder> GetStruct(int32 id);

	void Clear();
};

template<>
struct TStructOpsTypeTraits<FFINLuaProcessorStateStorage> : TStructOpsTypeTraitsBase2<FFINLuaProcessorStateStorage> {
	enum {
		WithStructuredSerializer = true,
    };
};