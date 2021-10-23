#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_Variable.generated.h"

UCLASS()
class UFIVSNode_Variable : public UFIVSScriptNode {
	GENERATED_BODY()

	UPROPERTY()
	FFIVSPinDataType Type;

	UPROPERTY()
	bool bAssignment = false;

	UPROPERTY()
	UFIVSPin* VarPin;
	
	UPROPERTY()
	UFIVSPin* ExecOutput = nullptr;

	UPROPERTY()
	UFIVSPin* DataInput;

public:
	// Begin UFIVSNode
	virtual void InitPins() override;
	virtual TArray<FFIVSNodeAction> GetNodeActions() const override;
	virtual FString GetNodeName() const override;
	// End UFIVSNode
	
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		TArray<UFIVSPin*> InputPins;
		if (DataInput) InputPins.Add(DataInput);
		return InputPins;
	}
	
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		if (bAssignment) {
			FFIVSValue Variable = Context.GetValue(VarPin);
			FFIVSValue Value = Context.GetValue(DataInput);
			*Variable = *Value;
			return ExecOutput;
		} else {
			FFINAnyNetworkValue* Value = Context.GetLocalVariable(GetFullName());
			if (!Value) {
				FFIVSValue InitValue = Context.GetValue(DataInput);
				Value = &Context.SetLocalVariable(GetFullName(), *InitValue); // TODO: Use different var name, current one is not persistable since Object it self doesnt get persisted but recreated
			}
			Context.SetValue(VarPin, FFIVSValue(Value));
			return nullptr;
		}
	}
};
