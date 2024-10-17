#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Variable.generated.h"

USTRUCT()
struct FFIVSNodeStatement_DeclareVariable : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid VarOut;
	UPROPERTY(SaveGame)
	FGuid InitialValue;

	FFIVSNodeStatement_DeclareVariable() = default;
	FFIVSNodeStatement_DeclareVariable(FGuid Node, FGuid VarOut, FGuid InitialValue) :
		FFIVSNodeStatement(Node),
		VarOut(VarOut),
		InitialValue(InitialValue) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	// End FFIVSNodeStatement
};

USTRUCT()
struct FFIVSNodeStatement_Assign : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecIn;
	UPROPERTY(SaveGame)
	FGuid ExecOut;
	UPROPERTY(SaveGame)
	FGuid VarIn;
	UPROPERTY(SaveGame)
	FGuid ValIn;

	FFIVSNodeStatement_Assign() = default;
	FFIVSNodeStatement_Assign(FGuid Node, FGuid ExecIn, FGuid ExecOut, FGuid VarIn, FGuid ValIn) :
		FFIVSNodeStatement(Node),
		ExecIn(ExecIn),
		ExecOut(ExecOut),
		VarIn(VarIn),
		ValIn(ValIn) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	// End FFIVSNodeStatement
};

UCLASS()
class UFIVSNode_Variable : public UFIVSScriptNode {
	GENERATED_BODY()

	UPROPERTY()
	FFIVSPinDataType Type;

	UPROPERTY()
	bool bAssignment = false;

	UPROPERTY()
	UFIVSPin* VarPin = nullptr;
	
	UPROPERTY()
	UFIVSPin* ExecInput = nullptr;

	UPROPERTY()
	UFIVSPin* ExecOutput = nullptr;

	UPROPERTY()
	UFIVSPin* DataInput = nullptr;

public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TFIRInstancedStruct<FFIVSNodeStatement> CreateNodeStatement() override;
	// End UFIVSScriptNode

	void SetType(const FFIVSPinDataType& InType, bool bIsAssignment);
};
