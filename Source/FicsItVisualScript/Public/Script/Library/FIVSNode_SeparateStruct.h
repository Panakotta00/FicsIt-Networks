#pragma once

#include "CoreMinimal.h"
#include "Algo/Accumulate.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSNode_SeparateStruct.generated.h"

USTRUCT()
struct FFIVSNodeStatement_SeparateStruct : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TMap<FString, FGuid> InputPins;
	UPROPERTY(SaveGame)
	TMap<FString, FGuid> OutputPins;
	UPROPERTY(SaveGame)
	UFIRStruct* Struct = nullptr;
	UPROPERTY(SaveGame)
	bool bBreak = false;

	FFIVSNodeStatement_SeparateStruct() = default;
	FFIVSNodeStatement_SeparateStruct(FGuid Node, TMap<FString, FGuid> InputPins, TMap<FString, FGuid> OutputPins, UFIRStruct* Struct, bool bBreak) :
		FFIVSNodeStatement(Node),
		InputPins(InputPins),
		OutputPins(OutputPins),
		Struct(Struct),
		bBreak(bBreak) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual bool IsVolatile() const override { return true; }
	// End FFIVSNodeStatement
};

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

	// Begin UFIVSScriptNode
	virtual TFIRInstancedStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		TMap<FString, FGuid> inputs;
		for (const auto& [key, pin] : InputPins) { inputs.Add(key, pin->PinId); }
		TMap<FString, FGuid> outputs;
		for (const auto& [key, pin] : OutputPins) { outputs.Add(key, pin->PinId); }
		return FFIVSNodeStatement_SeparateStruct{
			NodeId,
			inputs,
			outputs,
			Struct,
			bBreak,
		};
	}
	// End UFIVSScriptNode

	void SetStruct(UFIRStruct* InStruct);
};
