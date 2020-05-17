#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Processor/ProcessorStateStorage.h"
#include "Network/FINNetworkTrace.h"

#include "LuaProcessorStateStorage.generated.h"

class UFGPowerCircuit;
UCLASS()
class ULuaProcessorStateStorage : public UProcessorStateStorage {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TArray<FFINNetworkTrace> Traces;

	UPROPERTY(SaveGame)
	TArray<UObject*> References;

public:
	UPROPERTY(SaveGame)
	FString Thread;
	
	UPROPERTY(SaveGame)
	FString Globals;
	
    // Begin UProcessorStateStorage
	virtual void Serialize(FArchive& Ar) override;
	// End UProcessorStateStorage
			
	int32 Add(const FFINNetworkTrace& Trace);

	int32 Add(UObject* Ref);

	FFINNetworkTrace GetTrace(int32 id);

	UObject* GetRef(int32 id);
};
