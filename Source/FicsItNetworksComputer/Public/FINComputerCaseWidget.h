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

	// FIN-1.2-PORT: Bindung an Computer->OnEEPROMUpdate robust in C++ herstellen +
	// aktuellen EEPROM-Stand beim Oeffnen pushen. Behebt den "erstes Einsetzen wird
	// nicht live angezeigt"-Bug (BP-Bindung verpasste das erste Broadcast je nach
	// Construct-Timing). AddUniqueDynamic verhindert Doppel-Bindung mit dem BP.
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
