#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_SepperateStruct.generated.h"

UCLASS()
class UFIVSNode_SepperateStruct : public UFIVSScriptNode {
	GENERATED_BODY()

	UPROPERTY()
	UFINStruct* Struct = nullptr;
	UPROPERTY()
	bool bBreak = true;

	UPROPERTY()
	TMap<FString, UFIVSPin*> InputPins;
	UPROPERTY()
	TMap<FString, UFIVSPin*> OutputPins;
	
public:
	// Begin UFIVSNode
	virtual void InitPins() override;
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	virtual FString GetNodeName() const override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode
};
