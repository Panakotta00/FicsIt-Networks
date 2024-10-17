#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Branch.generated.h"

USTRUCT()
struct FFIVSNodeStatement_Branch : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecIn;
	UPROPERTY(SaveGame)
	FGuid ExecTrue;
	UPROPERTY(SaveGame)
	FGuid ExecFalse;
	UPROPERTY(SaveGame)
	FGuid Condition;

	FFIVSNodeStatement_Branch() = default;
	FFIVSNodeStatement_Branch(FGuid Node, FGuid ExecIn, FGuid ExecTrue, FGuid ExecFalse, FGuid Condition) :
		FFIVSNodeStatement(Node),
		ExecIn(ExecIn),
		ExecTrue(ExecTrue),
		ExecFalse(ExecFalse),
		Condition(Condition) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	// End FFIVSNodeStatement
};

UCLASS()
class UFIVSNode_Branch : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecTrue = nullptr;
	UPROPERTY()
	UFIVSPin* ExecFalse = nullptr;
	UPROPERTY()
	UFIVSPin* Condition = nullptr;

public:
	UFIVSNode_Branch();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TFIRInstancedStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		return FFIVSNodeStatement_Branch{
			NodeId,
			ExecIn->PinId,
			ExecTrue->PinId,
			ExecFalse->PinId,
			Condition->PinId,
		};
	}
	// End UFIVSScriptNode
};
