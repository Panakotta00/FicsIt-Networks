#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_DownCast.generated.h"

UCLASS()
class UFIVSNode_DownCast : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecInput = nullptr;
	UPROPERTY()
	UFIVSPin* DataInput = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOutput = nullptr;
	UPROPERTY()
	UFIVSPin* SuccessOutput = nullptr;
	UPROPERTY()
	UFIVSPin* DataOutput = nullptr;
	UPROPERTY()
	bool bPure = true;

	UPROPERTY()
	UFINClass* FromClass = nullptr;
	UPROPERTY()
	UFINClass* ToClass = nullptr;
	
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
