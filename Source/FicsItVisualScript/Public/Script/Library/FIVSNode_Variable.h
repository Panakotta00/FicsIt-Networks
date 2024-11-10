#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Variable.generated.h"

UCLASS()
class UFIVSNode_Variable : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
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

	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const override;
	// End IFVISCompileLuaInterface

	void SetType(const FFIVSPinDataType& InType, bool bIsAssignment);
};
