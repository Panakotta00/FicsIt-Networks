#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FGSignTypes.h"
#include "FicsItNetworksCustomVersion.h"
#include "Subsystem/ModSubsystem.h"
#include "Widgets/Layout/SBox.h"
#include "FINComputerSubsystem.generated.h"

class AFINComputerGPU;
class UFINGPUWidgetSign;
class AFGBuildableWidgetSign;
class AFGCharacterPlayer;
class UWidgetInteractionComponent;

UENUM()
enum EFINFSAlways {
	FIN_FS_Ask,
	FIN_FS_AlwaysOverride,
	FIN_FS_AlwaysKeep,
};

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINComputerSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TMap<AFGCharacterPlayer*, UWidgetInteractionComponent*> ScreenInteraction;

	UPROPERTY()
	TMap<AFINComputerGPU*, UFINGPUWidgetSign*> GPU2WidgetSign;
	UPROPERTY()
	TMap<UFINGPUWidgetSign*, AFINComputerGPU*> WidgetSign2GPU;

	UPROPERTY()
	class UEnhancedInputComponent* Input;

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

	static void SetFSAlways(EFINFSAlways InAlways);
	static EFINFSAlways GetFSAlways();
};

UCLASS()
class FICSITNETWORKSCOMPUTER_API UFINGPUSignPrefabWidget : public UFGSignPrefabWidget {
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