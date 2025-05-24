#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "FIVSScriptNode.h"
#include "FIRGlobalRegisterHelper.h"
#include "FIVSNode_LuaGeneric.generated.h"

struct FFIVSNodeLuaGenericPinMeta {
	FText DisplayName;
	EFIVSPinType PinType;
	FFIVSPinDataType ValueType;
};

struct FFIVSNodeLuaGenericMeta {
	FText Title;
	FString Symbol;
	FText Description;
	FText Category;
	FText SearchableText;
	TMap<FString, FFIVSNodeLuaGenericPinMeta> Pins;
};

UCLASS()
class UFIVSNode_LuaGeneric : public UFIVSScriptNode, public IFIVSCompileLuaInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFunction* Function = nullptr;
	UPROPERTY()
	FString Symbol;
	UPROPERTY()
	TArray<UFIVSPin*> GenericPins;

	static TMap<UClass*, TMap<FString, FFIVSNodeLuaGenericMeta>> FunctionMetaData;

public:
	static void RegisterMetaData(UClass* InClass, FString InFunctionName, FFIVSNodeLuaGenericMeta InMetaData) {
		FunctionMetaData.FindOrAdd(InClass).FindOrAdd(InFunctionName) = InMetaData;
	}

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style, class UFIVSEdEditor* Context) override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) override;
	// End UFIVSNode

	// Begin IFIVSCompileLuaInterface
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) override;
	// End IFVISCompileLuaInterface

	void SetFunction(UFunction* InFunction);
};

#define FIVSNode_LuaGenericPin(PinName, DisplayName, PinType, ValueType) \
	meta.Pins.Add(TEXT(PinName), FFIVSNodeLuaGenericPinMeta{FText::FromString(TEXT(DisplayName)), PinType, ValueType});

#define CAT(a,b) CAT2(a,b)
#define CAT2(a,b) a##b
#define FIVSNode_BeginLuaGenericMeta(FunctionName, Title, Symbol, Description, Category, ...) \
FFIRStaticGlobalRegisterFunc CAT(UFIVSNode_LuaGenericMeta, __COUNTER__) = FFIRStaticGlobalRegisterFunc([]() { \
	FString Name = TEXT(#FunctionName); \
	FFIVSNodeLuaGenericMeta meta{FText::FromString(TEXT(Title)), TEXT(Symbol), FText::FromString(TEXT(Description)), FText::FromString(TEXT(Category)), FText::FromString(TEXT(Title) TEXT(Symbol))};
#define FIVSNode_EndLuaGenericMeta() \
	UFIVSNode_LuaGeneric::RegisterMetaData(StaticClass(), Name, meta); \
});
