#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_SetProperty.generated.h"

USTRUCT()
struct FFIVSNodeStatement_SetProperty : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecIn;
	UPROPERTY(SaveGame)
	FGuid ExecOut;
	UPROPERTY(SaveGame)
	FGuid InstanceIn;
	UPROPERTY(SaveGame)
	FGuid DataIn;
	UPROPERTY(SaveGame)
	UFINProperty* Property = nullptr;

	FFIVSNodeStatement_SetProperty() = default;
	FFIVSNodeStatement_SetProperty(FGuid Node, FGuid ExecIn, FGuid ExecOut, FGuid InstanceIn, FGuid DataIn, UFINProperty* Property) :
		FFIVSNodeStatement(Node),
		ExecIn(ExecIn),
		ExecOut(ExecOut),
		InstanceIn(InstanceIn),
		DataIn(DataIn),
		Property(Property) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	// End FFIVSNodeStatement
};

UCLASS()
class UFIVSNode_SetProperty : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* InstanceIn = nullptr;
	UPROPERTY()
	UFIVSPin* DataIn = nullptr;

	UPROPERTY()
	UFINProperty* Property = nullptr;

public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNodes

	// Begin UFIVSGenericNode
	virtual TFINDynamicStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		return FFIVSNodeStatement_SetProperty{
			NodeId,
			ExecIn->PinId,
			ExecOut->PinId,
			InstanceIn->PinId,
			DataIn->PinId,
			Property,
		};
	}
	// End UFIVSGenericNode

	void SetProperty(UFINProperty* InProperty);
};
