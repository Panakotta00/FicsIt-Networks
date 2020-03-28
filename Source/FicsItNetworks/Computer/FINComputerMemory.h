#pragma once

#include "CoreMinimal.h"
#include "FINComputerModule.h"
#include "FINComputerMemory.generated.h"

UCLASS()
class AFINComputerMemory : public AFINComputerModule {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category="ComputerMemory")
		int MemoryCapacity;

	int GetCapacity();
};