#pragma once

#include "FIVSScriptNode.h"
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
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNode
	
	// Begin UFIVSScriptNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return "Branch"; }
	
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{ Condition };
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode
};
