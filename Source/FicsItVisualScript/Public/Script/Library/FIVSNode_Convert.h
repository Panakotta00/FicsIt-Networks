#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Convert.generated.h"

USTRUCT()
struct FFIVSNodeStatement_Convert : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid Input;
	UPROPERTY(SaveGame)
	FGuid Output;
	UPROPERTY(SaveGame)
	TEnumAsByte<EFINNetworkValueType> FromType;
	UPROPERTY(SaveGame)
	TEnumAsByte<EFINNetworkValueType> ToType;

	FFIVSNodeStatement_Convert() = default;
	FFIVSNodeStatement_Convert(FGuid Node, FGuid Input, FGuid Output, EFINNetworkValueType FromType, EFINNetworkValueType ToType) :
		FFIVSNodeStatement(Node),
		Input(Input),
		Output(Output),
		FromType(FromType),
		ToType(ToType) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual bool IsVolatile() const override { return true; }
	// End FFIVSNodeStatement
};

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
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNodes

	// Begin UFIVSGenericNode
	virtual TFINDynamicStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		return FFIVSNodeStatement_Convert{
			NodeId,
			Input->PinId,
			Output->PinId,
			FromType,
			ToType,
		};
	}
	// End UFIVSGenericNode

	void SetConversion(EFINNetworkValueType FromType, EFINNetworkValueType ToType);
};
