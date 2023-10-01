#pragma once

#include "FicsItNetworksCustomVersion.h"
#include "Subsystem/ModSubsystem.h"
#include "FGCharacterPlayer.h"
#include "FGSaveInterface.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Components/WidgetInteractionComponent.h"
#include "Patching/NativeHookManager.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "FINComputerSubsystem.generated.h"

class AFINComputerGPU;
class UFINGPUWidgetSign;

UENUM()
enum EFINFSAlways {
	FIN_FS_Ask,
	FIN_FS_AlwaysOverride,
	FIN_FS_AlwaysKeep,
};

USTRUCT()
struct FFINRailroadSwitchForce {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	int64 ForcedPosition;
	
	UPROPERTY(SaveGame)
	TArray<UFGRailroadTrackConnectionComponent*> ActualConnections;
};

USTRUCT()
struct FFINFactoryConnectorSettings {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TSubclassOf<UFGItemDescriptor> AllowedItem;

	UPROPERTY(SaveGame)
	bool bBlocked = false;

	UPROPERTY(SaveGame)
	int64 UnblockedTransfers;
};

UCLASS()
class FICSITNETWORKS_API AFINComputerSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TMap<AFGCharacterPlayer*, UWidgetInteractionComponent*> ScreenInteraction;

	UPROPERTY()
	TMap<AFINComputerGPU*, UFINGPUWidgetSign*> GPU2WidgetSign;
	UPROPERTY()
	TMap<UFINGPUWidgetSign*, AFINComputerGPU*> WidgetSign2GPU;

	UPROPERTY()
	UEnhancedInputComponent* Input;

	UPROPERTY(SaveGame, BlueprintReadOnly)
	TEnumAsByte<EFINCustomVersion> Version = EFINCustomVersion::FINBeforeCustomVersionWasAdded;

	int VirtualUserNum = 0;

	AFINComputerSubsystem();

	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float dt) override;
	// End AActor
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	void OnPrimaryFirePressed();
	void OnPrimaryFireReleased();
	void OnSecondaryFirePressed();
	void OnSecondaryFireReleased();

	UFUNCTION(BlueprintCallable, Category = "Computer", meta = (WorldContext = "WorldContext"))
	static AFINComputerSubsystem* GetComputerSubsystem(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Computer")
	void AttachWidgetInteractionToPlayer(AFGCharacterPlayer* character);
	
	UFUNCTION(BlueprintCallable, Category = "Computer")
	void DetachWidgetInteractionToPlayer(AFGCharacterPlayer* character);

	UFINGPUWidgetSign* AddGPUWidgetSign(AFINComputerGPU* GPU, AFGBuildableWidgetSign* BuildableSign);
	void DeleteGPUWidgetSign(AFINComputerGPU* GPU);

	void ForceRailroadSwitch(UFGRailroadTrackConnectionComponent* RailroadSwitch, int64 Track);
	FFINRailroadSwitchForce* GetForcedRailroadSwitch(UFGRailroadTrackConnectionComponent* RailroadSwitch);

	void AddRailroadSwitchConnection(CallScope<void(*)(UFGRailroadTrackConnectionComponent*,UFGRailroadTrackConnectionComponent*)>& Scope, UFGRailroadTrackConnectionComponent* Switch, UFGRailroadTrackConnectionComponent* Connection);
	void RemoveRailroadSwitchConnection(CallScope<void(*)(UFGRailroadTrackConnectionComponent*,UFGRailroadTrackConnectionComponent*)>& Scope, UFGRailroadTrackConnectionComponent* Switch, UFGRailroadTrackConnectionComponent* Connection);

	static void SetFSAlways(EFINFSAlways InAlways);
	static EFINFSAlways GetFSAlways();

	TOptional<TTuple<FCriticalSection&, FFINFactoryConnectorSettings&>> GetFactoryConnectorSettings(UFGFactoryConnectionComponent* InConnector);
	void SetFactoryConnectorAllowedItem(UFGFactoryConnectionComponent* InConnector, TSubclassOf<UFGItemDescriptor> InAllowedItem);
	TSubclassOf<UFGItemDescriptor> GetFactoryConnectorAllowedItem(UFGFactoryConnectionComponent* InConnector);
	void SetFactoryConnectorBlocked(UFGFactoryConnectionComponent* InConnector, bool bInBlocked);
	bool GetFactoryConnectorBlocked(UFGFactoryConnectionComponent* InConnector);
	int64 AddFactoryConnectorUnblockedTransfers(UFGFactoryConnectionComponent* InConnector, int64 InUnblockedTransfers);
	int64 GetFactoryConnectorUnblockedTransfers(UFGFactoryConnectionComponent* InConnector);
	
private:
	void UpdateRailroadSwitch(FFINRailroadSwitchForce& Force, UFGRailroadTrackConnectionComponent* Switch);
	void ForcedRailroadSwitchCleanup(FFINRailroadSwitchForce& Force, UFGRailroadTrackConnectionComponent* RailroadSwitch);

	void FactoryConnectorCleanup(UFGFactoryConnectionComponent* InConnector, FFINFactoryConnectorSettings& Settings);

	FCriticalSection ForcedRailroadSwitchesMutex;
	UPROPERTY(SaveGame)
	TMap<UFGRailroadTrackConnectionComponent*, FFINRailroadSwitchForce> ForcedRailroadSwitches;

	FCriticalSection FactoryConnectorSettingsMutex;
	UPROPERTY(SaveGame)
	TMap<UFGFactoryConnectionComponent*, FFINFactoryConnectorSettings> FactoryConnectorSettings;
};

UCLASS()
class UFINGPUSignPrefabWidget : public UFGSignPrefabWidget {
	GENERATED_BODY()
protected:
    // UWidget interface
    virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

public:
	UFUNCTION(BlueprintCallable)
	void OnNewWidget();

	UFUNCTION(BlueprintCallable)
	void OnNewGPU();

	void SetWidgetSign(UFINGPUWidgetSign* Sign);

private:
	TSharedPtr<SBox> Container = nullptr;

	UPROPERTY()
	UFINGPUWidgetSign* WidgetSign = nullptr;
};