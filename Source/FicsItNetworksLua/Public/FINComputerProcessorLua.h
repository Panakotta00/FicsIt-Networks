#pragma once

#include "Computer/FINComputerProcessor.h"
#include "FINComputerProcessorLua.generated.h"

UCLASS(Blueprintable)
class FICSITNETWORKSLUA_API AFINComputerProcessorLua : public AFINComputerProcessor {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	int LuaInstructionsPerTick = 1;
	
	// Begin AFINComputerProcessorLua
	virtual UFINKernelProcessor* CreateProcessor() override;
	// End AFINComputerProcessorLua
};