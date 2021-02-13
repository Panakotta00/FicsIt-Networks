#pragma once

#include "UserWidget.h"
#include "FicsItKernel/Processor/FINStateEEPROM.h"

#include "FINComputerCaseWidget.generated.h"

class AFINComputerCase;

UCLASS(Blueprintable)
class FICSITNETWORKS_API UFINComputerCaseWidget : public UUserWidget {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn="true"))
	AFINComputerCase* Computer = nullptr;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnEEPROMUpdate(AFINStateEEPROM* EEPROM);
};
