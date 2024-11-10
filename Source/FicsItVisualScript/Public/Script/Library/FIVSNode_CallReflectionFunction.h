#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_CallReflectionFunction.generated.h"

UCLASS()
class UFIVSNode_CallReflectionFunction : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
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
	UFIRFunction* Function = nullptr;
	
public:
	UFIVSNode_CallReflectionFunction();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNodes
	
	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const override;
	// End IFVISCompileLuaInterface

	void SetFunction(UFIRFunction* InFunction);
};