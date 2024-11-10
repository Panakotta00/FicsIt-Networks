#include "Script/Library/FIVSNode_LuaGeneric.h"

#include "DeclarativeSyntaxSupport.h"
#include "FIVSEdNodeViewer.h"

TMap<UClass*, TMap<FString, FFIVSNodeLuaGenericMeta>> UFIVSNode_LuaGeneric::FunctionMetaData;

void UFIVSNode_LuaGeneric::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for(TObjectIterator<UClass> It; It; ++It) {
		UClass* Class = *It;
		TMap<FString, FFIVSNodeLuaGenericMeta>* FuncMeta = FunctionMetaData.Find(Class);
		if (!FuncMeta) continue;
		for (TFieldIterator<UFunction> Func(Class); Func; ++Func) {
			FFIVSNodeLuaGenericMeta* Meta = FuncMeta->Find(Func->GetName());
			if (!Meta) continue;
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_LuaGeneric::StaticClass();
			Action.Title = Meta->Title;
			Action.Category = Meta->Category;
			Action.SearchableText = Meta->SearchableText;
			for (const auto& [_, pinMeta] : Meta->Pins) {
				Action.Pins.Add(FFIVSFullPinType(pinMeta.PinType, pinMeta.ValueType));
			}
			UFunction* F = *Func;
			Action.OnExecute.BindLambda([F](UFIVSNode* Node) {
				Cast<UFIVSNode_LuaGeneric>(Node)->SetFunction(F);
			});
			Actions.Add(Action);
		}
	}
}

TSharedRef<SFIVSEdNodeViewer> UFIVSNode_LuaGeneric::CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style) {
	return SNew(SFIVSEdOperatorNodeViewer, GraphViewer, this)
	.Style(Style)
	.Symbol(Symbol);
}

void UFIVSNode_LuaGeneric::SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const {
	Value->SetStringField(TEXT("Function"), Function->GetPathName());
}

void UFIVSNode_LuaGeneric::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) {
	SetFunction(Cast<UFunction>(FSoftObjectPath(Value->GetStringField(TEXT("Function"))).TryLoad()));
}

void UFIVSNode_LuaGeneric::CompileNodeToLua(FFIVSLuaCompilerContext& Context) const {
	TArray<FFIRAnyValue> Output;

	// allocate & initialize parameter struct
	uint8* ParamStruct = (uint8*)FMemory_Alloca(Function->PropertiesSize);
	for (TFieldIterator<FProperty> Prop(Function); Prop; ++Prop) {
		if (!(Prop->GetPropertyFlags() & CPF_Parm)) continue;
		if (!Prop->IsInContainer(Function->ParmsSize)) continue;
		Prop->InitializeValue_InContainer(ParamStruct);

		FString propName = Prop->GetName();
		if (propName == TEXT("Context")) {
			auto structProp = CastFieldChecked<FStructProperty>(*Prop);
			structProp->SetValue_InContainer(ParamStruct, &Context);
			continue;
		}
		if (Prop->GetPropertyFlags() & CPF_OutParm) continue;
		UFIVSPin* const* pin = Algo::FindByPredicate(GenericPins, [&propName](UFIVSPin* checkPin) {
			return checkPin->Name == propName;
		});
		if (!pin) continue;

		auto objProp = CastFieldChecked<FObjectProperty>(*Prop);
		objProp->SetValue_InContainer(ParamStruct, *pin);
	}

	UObject* Obj = const_cast<UObject*>(GetDefault<UObject>(Function->GetOuterUClass()));
	Obj->ProcessEvent(Function, ParamStruct);

	// destroy parameter struct
	for (TFieldIterator<FProperty> Prop(Function); Prop; ++Prop) {
		EPropertyFlags Flags = Prop->GetPropertyFlags();
		if (Flags & CPF_Parm) {
			if (!Prop->IsInContainer(Function->ParmsSize)) continue;

			FString propName = Prop->GetName();
			if (propName == TEXT("Context")) {
				auto structProp = CastFieldChecked<FStructProperty>(*Prop);
				structProp->GetValue_InContainer(ParamStruct, &Context);
			}

			Prop->DestroyValue_InContainer(ParamStruct);
		}
	}
}

void UFIVSNode_LuaGeneric::SetFunction(UFunction* InFunction) {
	Function = InFunction;

	TMap<FString, FFIVSNodeLuaGenericMeta>* FuncMeta = FunctionMetaData.Find(InFunction->GetOuterUClass());
	if (!FuncMeta) return;
	FFIVSNodeLuaGenericMeta* meta = FuncMeta->Find(Function->GetName());
	if (!meta) return;

	DisplayName = meta->Title;
	Symbol = meta->Symbol;

	DeletePins(GenericPins);
	GenericPins.Empty();

	for (const auto& [pinName, pinMeta] : meta->Pins) {
		GenericPins.Add(CreatePin(pinMeta.PinType, pinName, pinMeta.DisplayName, pinMeta.ValueType));
	}
}
