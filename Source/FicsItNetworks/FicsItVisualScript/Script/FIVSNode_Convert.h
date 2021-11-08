#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_Convert.generated.h"

UCLASS()
class UFIVSNode_Convert : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* Input = nullptr;
	UPROPERTY()
	UFIVSPin* Output = nullptr;

public:
	EFINNetworkValueType FromType = FIN_NIL;
	EFINNetworkValueType ToType = FIN_NIL;

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSNodes

	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return TEXT("Convert ") + FINGetNetworkValueTypeName(FromType) + TEXT(" to ") + FINGetNetworkValueTypeName(ToType); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode
};
