#pragma once

#include "CoreMinimal.h"
#include "Script/FIVSScriptNode.h"
#include "FINGlobalRegisterHelper.h"
#include "FIVSNode_UFunctionCall.generated.h"

struct FFIVSNodeUFunctionCallMeta {
	FText Title;
	FString Symbol;
	FText Description;
	FText Categrory;
	FText SearchableText;
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
	TArray<UFIVSPin*> Parameters;

	TMap<FProperty*, UFIVSPin*> PropertyToPin;

	static TMap<UClass*, TMap<FString, FFIVSNodeUFunctionCallMeta>> FunctionMetaData;
	
public:
	static void RegisterMetaData(UClass* InClass, FString InFunctionName, FFIVSNodeUFunctionCallMeta InMetaData) {
	    FunctionMetaData.FindOrAdd(InClass).FindOrAdd(InFunctionName) = InMetaData;
	}
	
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style) override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode

	void SetFunction(UFunction* InFunction, const FString& InSymbol);
};

#define CAT(a,b) CAT2(a,b)
#define CAT2(a,b) a##b
#define FIVSNode_UFunctionOperatorMeta(FunctionName, Title, Symbol, Description, Category) \
	FFINStaticGlobalRegisterFunc CAT(FIVSNode_UFunctionOperatorMeta, __COUNTER__) = FFINStaticGlobalRegisterFunc([]() { \
		UFIVSNode_UFunctionCall::RegisterMetaData(StaticClass(), TEXT(#FunctionName), {FText::FromString(TEXT(Title)), TEXT(Symbol), FText::FromString(TEXT(Description)), FText::FromString(TEXT(Category)), FText::FromString(TEXT(Title ## Symbol))}); \
	})
