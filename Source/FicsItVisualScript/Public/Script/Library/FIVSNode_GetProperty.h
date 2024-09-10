#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_GetProperty.generated.h"

UCLASS()
class UFIVSNode_GetProperty : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* InstanceIn = nullptr;
	UPROPERTY()
	UFIVSPin* DataOut = nullptr;

	UPROPERTY()
	UFINProperty* Property = nullptr;

public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSNodes

	// Begin UFIVSGenericNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode

	void SetProperty(UFINProperty* InProperty);
};