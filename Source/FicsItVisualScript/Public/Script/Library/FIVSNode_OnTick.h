#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_OnTick.generated.h"

UCLASS()
class UFIVSNode_OnTick : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	
public:
	UFIVSNode_OnTick();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNodes
	
	// Begin UFIVSGenericNode
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode
};