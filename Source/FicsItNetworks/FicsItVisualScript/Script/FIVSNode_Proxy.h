#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_Proxy.generated.h"

UCLASS()
class UFIVSNode_Proxy : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* AddrIn = nullptr;
	UPROPERTY()
	UFIVSPin* CompOut = nullptr;

public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNodes
	
	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return "Proxy"; }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{AddrIn};
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode
};