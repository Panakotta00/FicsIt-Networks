#pragma once

#include "CoreMinimal.h"
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
	void OnEEPROMUpdate(AFINStateEEPROM* EEPROM);
};
