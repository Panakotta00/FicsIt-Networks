#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Convert.generated.h"

UCLASS()
class UFIVSNode_Convert : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* Input = nullptr;
	UPROPERTY()
	UFIVSPin* Output = nullptr;

	UPROPERTY()
	TEnumAsByte<EFINNetworkValueType> FromType = FIN_NIL;
	UPROPERTY()
	TEnumAsByte<EFINNetworkValueType> ToType = FIN_NIL;

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

	void SetConversion(EFINNetworkValueType FromType, EFINNetworkValueType ToType);
};
