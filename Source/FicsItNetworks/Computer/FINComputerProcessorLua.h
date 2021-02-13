#pragma once

#include "CoreMinimal.h"
#include "FINComputerProcessor.h"
#include "FINComputerProcessorLua.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINComputerProcessorLua : public AFINComputerProcessor {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	int LuaInstructionsPerTick = 1;
	
	// Begin AFINComputerProcessorLua
	virtual FicsItKernel::Processor* CreateProcessor() override;
	// End AFINComputerProcessorLua
};