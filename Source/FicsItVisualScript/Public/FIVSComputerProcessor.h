#pragma once

#include "Computer/FINComputerProcessor.h"
#include "FIVSComputerProcessor.generated.h"

UCLASS(Blueprintable)
class AFINScriptProcessor : public AFINComputerProcessor {
	GENERATED_BODY()
public:
	// Begin AFINComputerProcessorLua
	virtual UFINKernelProcessor* CreateProcessor() override;
	// End AFINComputerProcessorLua
};
