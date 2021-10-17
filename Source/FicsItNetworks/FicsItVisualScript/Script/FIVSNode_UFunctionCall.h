#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_UFunctionCall.generated.h"

UCLASS()
class UFIVSNode_UFunctionCall : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFunction* Function = nullptr;

	TMap<FProperty*, UFIVSPin*> PropertyToPin;
	
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
