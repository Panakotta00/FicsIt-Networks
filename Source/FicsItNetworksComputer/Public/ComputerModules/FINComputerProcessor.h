#pragma once

#include "CoreMinimal.h"
#include "FINComputerModule.h"
#include "FINComputerProcessor.generated.h"

class UFINComputerCaseWidget;
class UFINKernelProcessor;

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINComputerProcessor : public AFINComputerModule {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	float KernelTicksPerSecond = 1.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UFINComputerCaseWidget> ComputerCaseWidget;
	
	virtual UFINKernelProcessor* CreateProcessor();
};