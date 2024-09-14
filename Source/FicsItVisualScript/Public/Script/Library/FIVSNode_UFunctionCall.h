#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FINGlobalRegisterHelper.h"
#include "FIVSUtils.h"
#include "FIVSNode_UFunctionCall.generated.h"

struct FFIVSNodeUFunctionCallMeta {
	FText Title;
	FString Symbol;
	FText Description;
	FText Categrory;
	FText SearchableText;
};

USTRUCT()
struct FFIVSNodeStatement_UFunctionCall : public FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ExecIn;
	UPROPERTY(SaveGame)
	FGuid ExecOut;
	UPROPERTY(SaveGame)
	TArray<FGuid> InputPins;
	UPROPERTY(SaveGame)
	TArray<FGuid> OutputPins;
	UPROPERTY(SaveGame)
	UFunction* Function = nullptr;
	UPROPERTY(SaveGame)
	TMap<FName, FGuid> PropertyToPin;

	FFIVSNodeStatement_UFunctionCall() = default;
	FFIVSNodeStatement_UFunctionCall(FGuid Node, FGuid ExecIn, FGuid ExecOut, const TArray<FGuid>& InputPins, const TArray<FGuid>& OutputPins, UFunction* Function, const TMap<FName, FGuid>& PropertyToPin) :
		FFIVSNodeStatement(Node),
		ExecIn(ExecIn),
		ExecOut(ExecOut),
		InputPins(InputPins),
		OutputPins(OutputPins),
		Function(Function),
		PropertyToPin(PropertyToPin) {}

	// Begin FFIVSNodeStatement
	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const override;
	virtual bool IsVolatile() const override { return true; }
	// End FFIVSNodeStatement
};

UCLASS()
class UFIVSNode_UFunctionCall : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFunction* Function = nullptr;

	UPROPERTY()
	FString Symbol;

	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;

	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;

	UPROPERTY()
	TArray<UFIVSPin*> Input;
	UPROPERTY()
	TArray<UFIVSPin*> Output;

	TMap<FProperty*, UFIVSPin*> PropertyToPin;

	static TMap<UClass*, TMap<FString, FFIVSNodeUFunctionCallMeta>> FunctionMetaData;
	
public:
	static void RegisterMetaData(UClass* InClass, FString InFunctionName, FFIVSNodeUFunctionCallMeta InMetaData) {
	    FunctionMetaData.FindOrAdd(InClass).FindOrAdd(InFunctionName) = InMetaData;
	}
	
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style) override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TFINDynamicStruct<FFIVSNodeStatement> CreateNodeStatement() override {
		TMap<FName, FGuid> propToGuid;

		for (auto [prop, pin] : PropertyToPin) {
			propToGuid.Add(prop->GetFName(), pin->PinId);
		}

		return FFIVSNodeStatement_UFunctionCall{
			NodeId,
			ExecIn ? ExecIn->PinId : FGuid(),
			ExecOut ? ExecOut->PinId : FGuid(),
			UFIVSUtils::GuidsFromPins(Input),
			UFIVSUtils::GuidsFromPins(Output),
			Function,
			propToGuid
		};
	}
	// End UFIVSScriptNode

	void SetFunction(UFunction* InFunction, const FString& InSymbol);
};

#define CAT(a,b) CAT2(a,b)
#define CAT2(a,b) a##b
#define FIVSNode_UFunctionOperatorMeta(FunctionName, Title, Symbol, Description, Category) \
	FFINStaticGlobalRegisterFunc CAT(FIVSNode_UFunctionOperatorMeta, __COUNTER__) = FFINStaticGlobalRegisterFunc([]() { \
		UFIVSNode_UFunctionCall::RegisterMetaData(StaticClass(), TEXT(#FunctionName), {FText::FromString(TEXT(Title)), TEXT(Symbol), FText::FromString(TEXT(Description)), FText::FromString(TEXT(Category)), FText::FromString(TEXT(Title ## Symbol))}); \
	})
