#pragma once

#include "CoreMinimal.h"
#include "FGDynamicStruct.h"
#include "Blueprint/UserWidget.h"
#include "FINComputerCaseWidget.generated.h"

class AFINComputerCase;

UCLASS(Blueprintable)
class FICSITNETWORKSCOMPUTER_API UFINComputerCaseWidget : public UUserWidget {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn="true"))
	class AFINComputerCase* Computer = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn="true"))
	UUserWidget* ComputerCaseInteractionWidget = nullptr;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnEEPROMUpdate(const FFGDynamicStruct& EEPROM);

	// FIN-1.2-PORT: Establish the binding to Computer->OnEEPROMUpdate robustly in C++ +
	// push the current EEPROM state on open. Fixes the "first insert is not shown
	// live" bug (the BP binding missed the first broadcast depending on Construct
	// timing). AddUniqueDynamic prevents a double binding with the BP.
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
