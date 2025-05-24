#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Literal.generated.h"

UCLASS()
class UFIVSNode_Literal : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
	GENERATED_BODY()

	UPROPERTY()
	TEnumAsByte<EFIRValueType> Type;

	UPROPERTY()
	UFIVSPin* Input = nullptr;
	
	UPROPERTY()
	UFIVSPin* Output = nullptr;

public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNode

	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) override;
	// End IFVISCompileLuaInterface

	void SetType(TEnumAsByte<EFIRValueType> InType);
};
