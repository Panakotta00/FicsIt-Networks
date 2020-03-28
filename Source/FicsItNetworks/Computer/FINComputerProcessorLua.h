#pragma once

#include "CoreMinimal.h"
#include "FINComputerProcessor.h"
#include "FINComputerProcessorLua.generated.h"

UCLASS()
class AFINComputerProcessorLua : public AFINComputerProcessor {
	GENERATED_BODY()
public:
	// Begin AFINComputerProcessorLua
	virtual FicsItKernel::Processor* CreateProcessor() override;
	// End AFINComputerProcessorLua
};