#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Branch.generated.h"

UCLASS()
class UFIVSNode_Branch : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
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

	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) override;
	// End IFVISCompileLuaInterface
};
