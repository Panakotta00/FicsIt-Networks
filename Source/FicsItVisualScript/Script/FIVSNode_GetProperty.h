#pragma once

#include "FIVSScriptNode.h"
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
	void SetProperty(UFINProperty* InProperty) {
		check(InstanceIn == nullptr);
		Property = InProperty;
	}

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSNodes

	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override {
		if (Property->GetPropertyFlags() & FIN_Prop_ClassProp) {
			return TEXT("Get ") + Property->GetInternalName() + TEXT(" (Class)");
		} else {
			return TEXT("Get ") + Property->GetInternalName();
		}
	}

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode
};