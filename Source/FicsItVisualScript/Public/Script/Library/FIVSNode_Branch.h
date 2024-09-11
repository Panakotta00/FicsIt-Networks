#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Branch.generated.h"

UCLASS()
class UFIVSNode_Branch : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecTrue = nullptr;
	UPROPERTY()
	UFIVSPin* ExecFalse = nullptr;
	UPROPERTY()
	UFIVSPin* Condition = nullptr;

public:
	UFIVSNode_Branch();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNode
	
	// Begin UFIVSScriptNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{ Condition };
	}

	virtual TArray<UFIVSPin*> ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode
};
