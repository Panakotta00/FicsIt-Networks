#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FGSubsystem.h"
#include "Queue.h"
#include "Engine/Engine.h"
#include "Network/FINNetworkTrace.h"

#include "FINComputerSubsystem.generated.h"

class UFGFactoryConnectionComponent;
class UFGPowerCircuit;

USTRUCT()
struct FFINFactoryHook {
	GENERATED_BODY()

	UPROPERTY()
    TArray<int32> Samples;
	
	UPROPERTY()
	int32 CountOfReferences = 0;

	UPROPERTY()
	int32 CurrentSample = 0;

	UPROPERTY()
	TSet<FFINNetworkTrace> Listeners;
	
	// Begin UObject
	bool Serialize(FArchive& Ar);
	// End UObject
	
	void Update();
	int32 GetSampleSum();
};

FArchive& operator<<(FArchive& Ar, FFINFactoryHook& Hook);

UCLASS()
class AFINComputerSubsystem : public AFGSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
	
public:
	UPROPERTY(SaveGame)
	TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, FFINFactoryHook> FactoryHooks;

	TMap<TWeakObjectPtr<UFGPowerCircuit>, TSet<FFINNetworkTrace>> PowerCircuitListeners;

	FCriticalSection MutexFactoryHooks;
	FCriticalSection MutexPowerCircuitListeners;
	FTimerHandle FactoryHookTimer;

	// Begin UObject
	virtual void Serialize(FArchive& Ar) override;
	// End UObject

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reson) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	UFUNCTION(BlueprintCallable, Category = "Computer", meta = (WorldContext = "WorldContext"))
	static AFINComputerSubsystem* GetComputerSubsystem(UObject* WorldContext);

	void UpdateHooks();
	void RemoveHook(FFINNetworkTrace Connector);
};
