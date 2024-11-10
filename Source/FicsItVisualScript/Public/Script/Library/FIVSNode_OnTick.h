#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_OnTick.generated.h"

USTRUCT()
struct FFIVSNodeStatement_OnTick : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecOut;

	FFIVSNodeStatement_OnTick() = default;
	FFIVSNodeStatement_OnTick(FGuid Node, FGuid ExecOut) :
		FFIVSNodeStatement(Node),
		ExecOut(ExecOut) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	// End FFIVSNodeStatement
};

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
	
	// Begin UFIVSGenericNode
	virtual TFIRInstancedStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		return FFIVSNodeStatement_OnTick{
			NodeId,
			ExecOut->PinId,
		};
	}
	// End UFIVSScriptNode

	// Begin IFIVSCompileLuaInterface
	virtual bool IsLuaRootNode() const { return true; }
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const override;
	// End IFVISCompileLuaInterface
};