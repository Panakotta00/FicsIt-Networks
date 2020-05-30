#pragma once
#include "FicsItKernel/Processor/FINStateEEPROM.h"
#include "FINStateEEPROMLua.generated.h"

UCLASS()
class AFINStateEEPROMLua : public AFINStateEEPROM {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, SaveGame)
	FString Code;
};
