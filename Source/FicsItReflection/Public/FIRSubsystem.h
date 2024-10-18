#pragma once

#include "CoreMinimal.h"
#include "Patching/NativeHookManager.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "FIRSubsystem.generated.h"

class UFGRailroadTrackConnectionComponent;
class UFGItemDescriptor;
class UFGFactoryConnectionComponent;

USTRUCT()
struct FICSITREFLECTION_API FFIRRailroadSwitchForce {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int64 ForcedPosition;

	UPROPERTY(SaveGame)
	TArray<UFGRailroadTrackConnectionComponent*> ActualConnections;
};

USTRUCT()
struct FICSITREFLECTION_API FFIRFactoryConnectorSettings {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TSubclassOf<UFGItemDescriptor> AllowedItem;

	UPROPERTY(SaveGame)
	bool bBlocked = false;

	UPROPERTY(SaveGame)
	int64 UnblockedTransfers;
};

UCLASS()
class FICSITREFLECTION_API AFIRSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Computer", meta = (WorldContext = "WorldContext"))
	static AFIRSubsystem* GetReflectionSubsystem(UObject* WorldContext);

	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface

	void ForceRailroadSwitch(UFGRailroadTrackConnectionComponent* RailroadSwitch, int64 Track);
	FFIRRailroadSwitchForce* GetForcedRailroadSwitch(UFGRailroadTrackConnectionComponent* RailroadSwitch);

	void AddRailroadSwitchConnection(CallScope<void(*)(UFGRailroadTrackConnectionComponent*,UFGRailroadTrackConnectionComponent*)>& Scope, UFGRailroadTrackConnectionComponent* Switch, UFGRailroadTrackConnectionComponent* Connection);
	void RemoveRailroadSwitchConnection(CallScope<void(*)(UFGRailroadTrackConnectionComponent*,UFGRailroadTrackConnectionComponent*)>& Scope, UFGRailroadTrackConnectionComponent* Switch, UFGRailroadTrackConnectionComponent* Connection);

	TOptional<TTuple<FCriticalSection&, FFIRFactoryConnectorSettings&>> GetFactoryConnectorSettings(UFGFactoryConnectionComponent* InConnector);
	void SetFactoryConnectorAllowedItem(UFGFactoryConnectionComponent* InConnector, TSubclassOf<UFGItemDescriptor> InAllowedItem);
	TSubclassOf<UFGItemDescriptor> GetFactoryConnectorAllowedItem(UFGFactoryConnectionComponent* InConnector);
	void SetFactoryConnectorBlocked(UFGFactoryConnectionComponent* InConnector, bool bInBlocked);
	bool GetFactoryConnectorBlocked(UFGFactoryConnectionComponent* InConnector);
	int64 AddFactoryConnectorUnblockedTransfers(UFGFactoryConnectionComponent* InConnector, int64 InUnblockedTransfers);
	int64 GetFactoryConnectorUnblockedTransfers(UFGFactoryConnectionComponent* InConnector);

private:
	void UpdateRailroadSwitch(FFIRRailroadSwitchForce& Force, UFGRailroadTrackConnectionComponent* Switch);
	void ForcedRailroadSwitchCleanup(FFIRRailroadSwitchForce& Force, UFGRailroadTrackConnectionComponent* RailroadSwitch);

	void FactoryConnectorCleanup(UFGFactoryConnectionComponent* InConnector, FFIRFactoryConnectorSettings& Settings);

	FCriticalSection ForcedRailroadSwitchesMutex;
	UPROPERTY(SaveGame)
	TMap<UFGRailroadTrackConnectionComponent*, FFIRRailroadSwitchForce> ForcedRailroadSwitches;

	FCriticalSection FactoryConnectorSettingsMutex;
	UPROPERTY(SaveGame)
	TMap<UFGFactoryConnectionComponent*, FFIRFactoryConnectorSettings> FactoryConnectorSettings;
};