#pragma once

#include "GameFramework/Actor.h"
#include "FGSaveInterface.h"
#include "FINStateEEPROM.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINEEPROMUpdateDelegate);

UCLASS()
class AFINStateEEPROM : public AActor, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FFINEEPROMUpdateDelegate UpdateDelegate;
	
	AFINStateEEPROM();
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	UFUNCTION(NetMulticast, Unreliable)
    void OnCodeUpdate();
};
