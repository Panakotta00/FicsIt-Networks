#pragma once

#include "CoreMinimal.h"
#include "FINLabelContainerInterface.h"
#include "FINItemStateEEPROM.generated.h"

USTRUCT(BlueprintType)
struct FICSITNETWORKSCOMPUTER_API FFINItemStateEEPROM : public FFINLabelContainerInterface {
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString Label;

	// Begin FFINLabelContainerInterface
	virtual FString GetLabel() const override { return Label; }
	virtual void SetLabel(const FString& InLabel) override { Label = InLabel; }
	// End FFINLabelContainerInterface
};
