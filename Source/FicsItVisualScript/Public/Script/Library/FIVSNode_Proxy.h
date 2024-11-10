#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Proxy.generated.h"

USTRUCT()
struct FFIVSNodeStatement_Proxy : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecIn;
	UPROPERTY(SaveGame)
	FGuid ExecOut;
	UPROPERTY(SaveGame)
	FGuid AddrIn;
	UPROPERTY(SaveGame)
	FGuid CompOut;

	FFIVSNodeStatement_Proxy() = default;
	FFIVSNodeStatement_Proxy(FGuid Node, FGuid ExecIn, FGuid ExecOut, FGuid AddrIn, FGuid CompOut) :
		FFIVSNodeStatement(Node),
		ExecIn(ExecIn),
		ExecOut(ExecOut),
		AddrIn(AddrIn),
		CompOut(CompOut) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	// End FFIVSNodeStatement
};

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
	
	// Begin UFIVSGenericNode
	virtual TFIRInstancedStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		return FFIVSNodeStatement_Proxy{
			NodeId,
			ExecIn->PinId,
			ExecOut->PinId,
			AddrIn->PinId,
			CompOut->PinId
		};
	}
	// End UFIVSGenericNode

	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const override;
	// End IFVISCompileLuaInterface
};
