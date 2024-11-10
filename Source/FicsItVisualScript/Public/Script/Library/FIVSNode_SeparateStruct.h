#pragma once

#include "CoreMinimal.h"
#include "Algo/Accumulate.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_SeparateStruct.generated.h"

UCLASS()
class UFIVSNode_SeparateStruct : public UFIVSScriptNode {
	GENERATED_BODY()

	UPROPERTY()
	UFIRStruct* Struct = nullptr;
	UPROPERTY()
	bool bBreak = true;

	UPROPERTY()
	TMap<FString, UFIVSPin*> InputPins;
	UPROPERTY()
	TMap<FString, UFIVSPin*> OutputPins;
	
public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNode

	void SetStruct(UFIRStruct* InStruct);
};
