#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_CallReflectionFunction.generated.h"


UCLASS()
class UFIVSNode_CallReflectionFunction : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* Self = nullptr;
	UPROPERTY()
	TArray<UFIVSPin*> InputPins;
	UPROPERTY()
	TArray<UFIVSPin*> OutputPins;

	UPROPERTY()
	UFINFunction* Function = nullptr;
	
public:
	UFIVSNode_CallReflectionFunction();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSNodes
	
	// Begin UFIVSScriptNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual TArray<UFIVSPin*> ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode

	void SetFunction(UFINFunction* InFunction);
};