#pragma once

#include "FicsItKernel/Processor/FINStateEEPROM.h"
#include "Script/FIVSGraph.h"
#include "FIVSStateEEPROM.generated.h"

UCLASS()
class AFIVSStateEEPROM : public AFINStateEEPROM {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame, BlueprintReadOnly)
	UFIVSGraph* Graph = nullptr;

	AFIVSStateEEPROM();
};
