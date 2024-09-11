#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_Sequence.generated.h"

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
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style) override;
	virtual TArray<UFIVSPin*> ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSScriptNode

	void SetOutputNum(int32 OutputNum);
};
