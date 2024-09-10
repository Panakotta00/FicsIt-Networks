#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Print.generated.h"

UCLASS()
class UFIVSNode_Print : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* MessageIn = nullptr;

public:
	UFIVSNode_Print();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNodes
	
	// Begin UFIVSGenericNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode
};
