#pragma once

#include "CoreMinimal.h"
#include "FIVSUtils.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Sequence.generated.h"

USTRUCT()
struct FFIVSNodeStatement_Sequence : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecIn;
	UPROPERTY(SaveGame)
	TArray<FGuid> ExecOut;

	FFIVSNodeStatement_Sequence() = default;
	FFIVSNodeStatement_Sequence(FGuid Node, FGuid ExecIn, const TArray<FGuid>& ExecOut) :
		FFIVSNodeStatement(Node),
		ExecIn(ExecIn),
		ExecOut(ExecOut) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	// End FFIVSNodeStatement
};


UCLASS()
class UFIVSNode_Sequence : public UFIVSScriptNode {
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
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style) override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	virtual TFINDynamicStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		return FFIVSNodeStatement_Sequence{
			NodeId,
			ExecIn->PinId,
			UFIVSUtils::GuidsFromPins(ExecOut),
		};
	}
	// End UFIVSScriptNode

	void SetOutputNum(int32 OutputNum);
};
