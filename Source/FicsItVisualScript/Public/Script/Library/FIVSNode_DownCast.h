#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_DownCast.generated.h"

class UFINClass;

USTRUCT()
struct FFIVSNodeStatement_Cast : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecIn;
	UPROPERTY(SaveGame)
	FGuid SuccessOut;
	UPROPERTY(SaveGame)
	FGuid FailureOut;
	UPROPERTY(SaveGame)
	FGuid DataIn;
	UPROPERTY(SaveGame)
	FGuid DataOut;
	UPROPERTY(SaveGame)
	UFINClass* ToClass;

	FFIVSNodeStatement_Cast() = default;
	FFIVSNodeStatement_Cast(FGuid Node, FGuid ExecIn, FGuid SuccessOut, FGuid FailureOut, FGuid DataIn, FGuid DataOut, UFINClass* ToClass) :
		FFIVSNodeStatement(Node),
		ExecIn(ExecIn),
		SuccessOut(SuccessOut),
		FailureOut(FailureOut),
		DataIn(DataIn),
		DataOut(DataOut),
		ToClass(ToClass) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual bool IsVolatile() const override { return true; }
	// End FFIVSNodeStatement
};

UCLASS()
class UFIVSNode_DownCast : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecInput = nullptr;
	UPROPERTY()
	UFIVSPin* SuccessOutput = nullptr;
	UPROPERTY()
	UFIVSPin* FailureOutput = nullptr;
	UPROPERTY()
	UFIVSPin* DataInput = nullptr;
	UPROPERTY()
	UFIVSPin* DataOutput = nullptr;
	UPROPERTY()
	bool bPure = true;

	UPROPERTY()
	UFINClass* ToClass = nullptr;
	
public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNode
	
	// Begin UFIVSScriptNode
	virtual TFINDynamicStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		return FFIVSNodeStatement_Cast{
			NodeId,
			ExecInput ? ExecInput->PinId : FGuid(),
			SuccessOutput ? SuccessOutput->PinId : FGuid(),
			FailureOutput ? FailureOutput->PinId : FGuid(),
			DataInput->PinId,
			DataOutput->PinId,
			ToClass,
		};
	}
	// End UFIVSScriptNode

	void SetClass(UFINClass* ToClass);
};
