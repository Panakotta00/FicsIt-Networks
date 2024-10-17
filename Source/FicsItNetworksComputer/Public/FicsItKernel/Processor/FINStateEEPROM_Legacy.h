#pragma once

#include "CoreMinimal.h"
#include "FGLegacyItemStateActorInterface.h"
#include "FGSaveInterface.h"
#include "GameFramework/Actor.h"
#include "FINStateEEPROM_Legacy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINEEPROMUpdateDelegate);

UCLASS(Abstract)
class FICSITNETWORKSCOMPUTER_API AFINStateEEPROM_Legacy : public AActor, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame, Replicated)
	FString Label;

	AFINStateEEPROM_Legacy();

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
};
