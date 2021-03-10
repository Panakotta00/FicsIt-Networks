#pragma once

#include "CoreMinimal.h"
#include "FINComputerCaseWidget.h"
#include "FINComputerModule.h"
#include "FicsItKernel/Processor/Processor.h"
#include "FINComputerProcessor.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINComputerProcessor : public AFINComputerModule {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	float KernelTicksPerSecond = 1.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UFINComputerCaseWidget> ComputerCaseWidget;
	
	virtual UFINKernelProcessor* CreateProcessor();
};