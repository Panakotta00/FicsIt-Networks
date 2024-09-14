#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_CallReflectionFunction.generated.h"

USTRUCT()
struct FFIVSNodeStatement_CallReflectionFunction : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecIn;
	UPROPERTY(SaveGame)
	FGuid ExecOut;
	UPROPERTY(SaveGame)
	FGuid Self;
	UPROPERTY(SaveGame)
	TArray<FGuid> InputPins;
	UPROPERTY(SaveGame)
	TArray<FGuid> OutputPins;
	UPROPERTY(SaveGame)
	UFINFunction* Function;

	FFIVSNodeStatement_CallReflectionFunction() = default;
	FFIVSNodeStatement_CallReflectionFunction(FGuid Node, FGuid ExecIn, FGuid ExecOut, FGuid Self, const TArray<FGuid>& InputPins, const TArray<FGuid>& OutputPins, UFINFunction* Function) :
		FFIVSNodeStatement(Node),
		ExecIn(ExecIn),
		ExecOut(ExecOut),
		Self(Self),
		InputPins(InputPins),
		OutputPins(OutputPins),
		Function(Function) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	// End FFIVSNodeStatement
};

UCLASS()
class UFIVSNode_CallReflectionFunction : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* Self = nullptr;
	UPROPERTY()
	TArray<UFIVSPin*> InputPins;
	UPROPERTY()
	TArray<UFIVSPin*> OutputPins;

	UPROPERTY()
	UFINFunction* Function = nullptr;
	
public:
	UFIVSNode_CallReflectionFunction();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNodes
	
	// Begin UFIVSScriptNode
	virtual TFINDynamicStruct<FFIVSNodeStatement> CreateNodeStatement() override;
	// End UFIVSScriptNode

	void SetFunction(UFINFunction* InFunction);
};