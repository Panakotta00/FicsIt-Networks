#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_MathOperation.generated.h"

UCLASS()
class UFIVSNode_MathOperation : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* DataInput = nullptr;
	UPROPERTY()
	UFIVSPin* DataOutput = nullptr;
	
public:
	// Begin UFIVSNode
	virtual TArray<FFIVSNodeAction> GetNodeActions() const override;
	virtual void InitPins() override;
	// End UFIVSNode
	
	// Begin UFIVSScriptNode
	virtual FString GetNodeName() const override;
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode
};
