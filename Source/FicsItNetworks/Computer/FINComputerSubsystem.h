#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGCharacterPlayer.h"
#include "FGSaveInterface.h"
#include "Components/WidgetInteractionComponent.h"
#include "FicsItNetworks/FicsItNetworksCustomVersion.h"

#include "FINComputerSubsystem.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINComputerSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
	
public:
	UPROPERTY(SaveGame, BlueprintReadOnly)
	TEnumAsByte<EFINCustomVersion> Version = EFINCustomVersion::FINBeforeCustomVersionWasAdded;

	AFINComputerSubsystem();
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	UFUNCTION(BlueprintCallable, Category = "Computer", meta = (WorldContext = "WorldContext"))
	static AFINComputerSubsystem* GetComputerSubsystem(UObject* WorldContext);
};
