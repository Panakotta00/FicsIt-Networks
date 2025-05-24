#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_OnTick.generated.h"

UCLASS()
class UFIVSNode_OnTick : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	
public:
	UFIVSNode_OnTick();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNodes
	
	// Begin IFIVSCompileLuaInterface
	virtual bool IsLuaRootNode() const { return true; }
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) override;
	// End IFVISCompileLuaInterface
};