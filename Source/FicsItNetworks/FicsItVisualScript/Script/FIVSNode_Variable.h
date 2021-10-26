#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_Variable.generated.h"

UCLASS()
class UFIVSNode_Variable : public UFIVSScriptNode {
	GENERATED_BODY()

	UPROPERTY()
	FFIVSPinDataType Type;

	UPROPERTY()
	bool bAssignment = false;

	UPROPERTY()
	UFIVSPin* VarPin;
	
	UPROPERTY()
	UFIVSPin* ExecOutput = nullptr;

	UPROPERTY()
	UFIVSPin* DataInput;

public:
	// Begin UFIVSNode
	virtual void InitPins() override;
	virtual TArray<FFIVSNodeAction> GetNodeActions() const override;
	virtual FString GetNodeName() const override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode
};
