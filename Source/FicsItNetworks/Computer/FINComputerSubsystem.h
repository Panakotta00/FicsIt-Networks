#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FGSubsystem.h"
#include "FicsItNetworksCustomVersion.h"
#include "Queue.h"
#include "WidgetInteractionComponent.h"
#include "Engine/Engine.h"
#include "Network/FINNetworkTrace.h"

#include "FINComputerSubsystem.generated.h"

UCLASS()
class AFINComputerSubsystem : public AFGSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	UWidgetInteractionComponent* ScreenInteraction;

	UPROPERTY()
	UInputComponent* Input;

	UPROPERTY(SaveGame, BlueprintReadOnly)
	TEnumAsByte<EFINCustomVersion> Version = EFINCustomVersion::FINBeforeCustomVersionWasAdded;

	AFINComputerSubsystem();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float dt) override;
	// End AActor
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	void OnPrimaryFirePressed();
	void OnPrimaryFireReleased();
	void OnSecondaryFirePressed();
	void OnSecondaryFireReleased();

	UFUNCTION(BlueprintCallable, Category = "Computer", meta = (WorldContext = "WorldContext"))
	static AFINComputerSubsystem* GetComputerSubsystem(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Computer")
	UWidgetInteractionComponent* AttachWidgetInteractionToPlayer(AFGCharacterPlayer* character);
};
