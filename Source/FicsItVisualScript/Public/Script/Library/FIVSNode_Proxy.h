#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Proxy.generated.h"

UCLASS()
class UFIVSNode_Proxy : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* AddrIn = nullptr;
	UPROPERTY()
	UFIVSPin* CompOut = nullptr;

public:
	UFIVSNode_Proxy();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	// End UFIVSNodes
	
	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) override;
	// End IFVISCompileLuaInterface
};
