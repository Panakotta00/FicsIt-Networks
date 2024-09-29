#pragma once

#include "CoreMinimal.h"
#include "FINLabelContainerInterface.h"
#include "FINStateEEPROM.generated.h"

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINStateEEPROM : public FFINLabelContainerInterface {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FString Label;

	// Begin FFINLabelContainerInterface
	virtual FString GetLabel() const override { return Label; }
	virtual void SetLabel(const FString& InLabel) override { Label = InLabel; }
	// End FFINLabelContainerInterface
};
