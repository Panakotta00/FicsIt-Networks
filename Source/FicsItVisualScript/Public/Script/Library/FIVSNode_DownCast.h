#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_DownCast.generated.h"

class UFIRClass;

UCLASS()
class UFIVSNode_DownCast : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
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
	UFIRClass* ToClass = nullptr;
	
public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNode
	
	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const override;
	// End IFVISCompileLuaInterface

	void SetClass(UFIRClass* ToClass);
};
