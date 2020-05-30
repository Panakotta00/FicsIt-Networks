#pragma once

#include "GameFramework/Actor.h"
#include "FGSaveInterface.h"
#include "FINStateEEPROM.generated.h"

UCLASS()
class AFINStateEEPROM : public AActor, public IFGSaveInterface {
	GENERATED_BODY()
public:
	AFINStateEEPROM();
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
};
