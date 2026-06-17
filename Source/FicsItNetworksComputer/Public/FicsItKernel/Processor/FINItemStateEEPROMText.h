#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Processor/FINItemStateEEPROM.h"
#include "FINItemStateEEPROMText.generated.h"

USTRUCT(BlueprintType)
struct FICSITNETWORKSCOMPUTER_API FFINItemStateEEPROMText : public FFINItemStateEEPROM {
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString Code;
};
