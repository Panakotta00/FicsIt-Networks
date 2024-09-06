#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FINLabelContainerInterface.h"
#include "GameFramework/Actor.h"
#include "FINStateEEPROM.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINEEPROMUpdateDelegate);

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINStateEEPROM : public AActor, public IFGSaveInterface, public IFINLabelContainerInterface {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame, Replicated)
	FString Label;

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

	// Begin IFINLabelContainerInterface
	FString GetLabel_Implementation() override { return Label; }
	void SetLabel_Implementation(const FString& InLabel) override { Label = InLabel; }
	// End IFINLabelContainerInterface

	UFUNCTION(NetMulticast, Unreliable)
    void OnCodeUpdate();

	UFUNCTION(BlueprintCallable, Category="Computer")
	virtual bool CopyDataTo(AFINStateEEPROM* InFrom) { return false; }
};
