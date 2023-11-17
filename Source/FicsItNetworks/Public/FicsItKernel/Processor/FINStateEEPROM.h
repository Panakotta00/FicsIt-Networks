#pragma once

#include "FGSaveInterface.h"
#include "GameFramework/Actor.h"
#include "FINStateEEPROM.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINEEPROMUpdateDelegate);

UCLASS()
class FICSITNETWORKS_API AFINStateEEPROM : public AActor, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FFINEEPROMUpdateDelegate UpdateDelegate;

	UPROPERTY()
	bool bShouldUpdate = false;
	
	AFINStateEEPROM();

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	UFUNCTION(NetMulticast, Unreliable)
    void OnCodeUpdate();

	UFUNCTION(BlueprintCallable, Category="Computer")
	virtual bool CopyDataTo(AFINStateEEPROM* InFrom) { return false; }
};
