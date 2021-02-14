#pragma once

#include "CoreMinimal.h"

#include "FGCharacterPlayer.h"
#include "FGSaveInterface.h"
#include "FGSubsystem.h"
#include "FicsItNetworksCustomVersion.h"
#include "WidgetInteractionComponent.h"

#include "FINComputerSubsystem.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINComputerSubsystem : public AFGSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TMap<AFGCharacterPlayer*, UWidgetInteractionComponent*> ScreenInteraction;

	UPROPERTY()
	UInputComponent* Input;

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
};
