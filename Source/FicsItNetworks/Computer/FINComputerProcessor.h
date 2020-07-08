#pragma once

#include "CoreMinimal.h"
#include "FINComputerModule.h"
#include "FicsItKernel/Processor/Processor.h"
#include "FINComputerProcessor.generated.h"

UCLASS()
class AFINComputerProcessor : public AFINComputerModule {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	float KernelTicksPerSecond = 1.0;
	
	virtual FicsItKernel::Processor* CreateProcessor();
};