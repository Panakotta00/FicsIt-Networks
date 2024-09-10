#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
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
	UFIVSPin* ExecInput = nullptr;

	UPROPERTY()
	UFIVSPin* ExecOutput = nullptr;

	UPROPERTY()
	UFIVSPin* DataInput;

public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode

	void SetType(const FFIVSPinDataType& InType, bool bIsAssignment);
};
