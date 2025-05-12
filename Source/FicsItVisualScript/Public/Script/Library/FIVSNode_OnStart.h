#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_OnStart.generated.h"

UCLASS()
class UFIVSNode_OnStart : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	
public:
	UFIVSNode_OnStart();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNodes
	
	// Begin IFIVSCompileLuaInterface
	virtual bool IsLuaRootNode() const { return true; }
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const override;
	// End IFVISCompileLuaInterface
};