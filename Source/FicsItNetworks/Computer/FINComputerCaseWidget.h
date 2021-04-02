#pragma once

#include "Blueprint/UserWidget.h"
#include "FicsItNetworks/FicsItKernel/Processor/FINStateEEPROM.h"

#include "FINComputerCaseWidget.generated.h"

class AFINComputerCase;

UCLASS(Blueprintable)
class FICSITNETWORKS_API UFINComputerCaseWidget : public UUserWidget {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn="true"))
	AFINComputerCase* Computer = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn="true"))
	UUserWidget* ComputerCaseInteractionWidget = nullptr;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnEEPROMUpdate(AFINStateEEPROM* EEPROM);
};
