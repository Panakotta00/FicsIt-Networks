#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_SeparateStruct.generated.h"

UCLASS()
class UFIVSNode_SeparateStruct : public UFIVSScriptNode {
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
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual TArray<UFIVSPin*> ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode

	void SetStruct(UFINStruct* InStruct);
};
