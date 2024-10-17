#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_GetProperty.generated.h"

USTRUCT()
struct FFIVSNodeStatement_GetProperty : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid DataIn;
	UPROPERTY(SaveGame)
	FGuid DataOut;
	UPROPERTY(SaveGame)
	UFIRProperty* Property;

	FFIVSNodeStatement_GetProperty() = default;
	FFIVSNodeStatement_GetProperty(FGuid Node, FGuid DataIn, FGuid DataOut, UFIRProperty* Property) :
		FFIVSNodeStatement(Node),
		DataIn(DataIn),
		DataOut(DataOut),
		Property(Property) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual bool IsVolatile() const override { return true; }
	// End FFIVSNodeStatement
};

UCLASS()
class UFIVSNode_GetProperty : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* InstanceIn = nullptr;
	UPROPERTY()
	UFIVSPin* DataOut = nullptr;

	UPROPERTY()
	UFIRProperty* Property = nullptr;

public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNodes

	// Begin UFIVSGenericNode
	virtual TFIRInstancedStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		return FFIVSNodeStatement_GetProperty{
			NodeId,
			InstanceIn->PinId,
			DataOut->PinId,
			Property,
		};
	}
	// End UFIVSGenericNode

	void SetProperty(UFIRProperty* InProperty);
};
