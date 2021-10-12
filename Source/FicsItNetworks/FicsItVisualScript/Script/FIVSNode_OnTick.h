#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_OnTick.generated.h"

UCLASS()
class UFIVSNode_OnTick : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	
public:
	// Begin UFIVSNode
	virtual TArray<FFIVSNodeAction> GetNodeActions() const override;
	// End UFIVSNodes
	
	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return "Event Tick"; }
	
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode
};