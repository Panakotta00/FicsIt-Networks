#pragma once

#include "CoreMinimal.h"
#include "AABB.h"
#include "AABB.h"
#include "AABB.h"
#include "AABB.h"
#include "FIVSCompileLua.h"
#include "FIVSUtils.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Sequence.generated.h"

UCLASS()
class UFIVSNode_Sequence : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	TArray<UFIVSPin*> ExecOut;

public:
	UFIVSNode_Sequence();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void ExtendPinContextMenu(UFIVSPin* InPin, FMenuBuilder& MenuBuilder) override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style, class UFIVSEdEditor* Context) override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSScriptNode

	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const override;
	// End IFVISCompileLuaInterface

	void SetOutputNum(int32 OutputNum);
};
