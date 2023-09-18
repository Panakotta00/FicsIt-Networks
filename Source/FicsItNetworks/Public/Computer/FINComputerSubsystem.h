#pragma once

#include "FicsItNetworksCustomVersion.h"
#include "Subsystem/ModSubsystem.h"
#include "FGCharacterPlayer.h"
#include "FGSaveInterface.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Components/WidgetInteractionComponent.h"
#include "FINComputerSubsystem.generated.h"

class AFINComputerGPU;
class UFINGPUWidgetSign;
class UFGRailroadTrackConnectionComponent;

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
	int64 GetForcedRailroadSwitch(UFGRailroadTrackConnectionComponent* RailroadSwitch);

private:
	UPROPERTY(SaveGame)
	TMap<UFGRailroadTrackConnectionComponent*, int64> ForcedRailroadSwitches;
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