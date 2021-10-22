#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_SetProperty.generated.h"

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
	void SetProperty(UFINProperty* InProperty) {
		check(ExecIn == nullptr);
		Property = InProperty;
	}

	// Begin UFIVSNode
	virtual TArray<FFIVSNodeAction> GetNodeActions() const override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSNodes

	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override {
		if (Property->GetPropertyFlags() & FIN_Prop_ClassProp) {
			return TEXT("Set ") + Property->GetInternalName() + TEXT(" (Class)");
		} else {
			return TEXT("Set ") + Property->GetInternalName();
		}
	}

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode
};
