#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Processor/ProcessorStateStorage.h"
#include "Network/FINNetworkTrace.h"

#include "LuaProcessorStateStorage.generated.h"

#define MakeDynamicStruct(Type, ...) MakeShared<FDynamicStructHolder>(new Type(__VA_ARGS__))

USTRUCT()
struct FDynamicStructHolder {
	GENERATED_BODY()
	
private:
	void* Data = nullptr;
	UStruct* Struct = nullptr;

public:
	FDynamicStructHolder();
	FDynamicStructHolder(UStruct* Struct);
	~FDynamicStructHolder();
	
	bool Serialize(FArchive& Ar);

	UStruct* GetStruct();
	void* GetData();

	template<typename T>
	T& Get() {
		return *static_cast<T*>(GetData());
	}
};

inline void operator<<(FArchive& Ar, FDynamicStructHolder& Struct) {
	Struct.Serialize(Ar);
}

USTRUCT()
struct FFINBoolData {
	GENERATED_BODY()
	
    bool Data;
	
	inline bool Serialize(FArchive& Ar) {
		Ar << Data;
		return true;
	}
};

inline void operator<<(FArchive& Ar, FFINBoolData& Data) {
	Data.Serialize(Ar);
}

class UFGPowerCircuit;
UCLASS()
class ULuaProcessorStateStorage : public UProcessorStateStorage {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TArray<FFINNetworkTrace> Traces;

	UPROPERTY(SaveGame)
	TArray<UObject*> References;

	TArray<TSharedPtr<FDynamicStructHolder>> Structs;

public:
	UPROPERTY(SaveGame)
	FString Thread;
	
	UPROPERTY(SaveGame)
	FString Globals;

	UPROPERTY(SaveGame)
	int PullState = 0;

	UPROPERTY(SaveGame)
    double Timeout = 0;

	UPROPERTY(SaveGame)
    uint64 PullStart = 0;

    // Begin UProcessorStateStorage
	virtual void Serialize(FArchive& Ar) override;
	// End UProcessorStateStorage
			
	int32 Add(const FFINNetworkTrace& Trace);

	int32 Add(UObject* Ref);

	int32 Add(TSharedPtr<FDynamicStructHolder> Struct);

	FFINNetworkTrace GetTrace(int32 id);

	UObject* GetRef(int32 id);

	TSharedPtr<FDynamicStructHolder> GetStruct(int32 id);
};
