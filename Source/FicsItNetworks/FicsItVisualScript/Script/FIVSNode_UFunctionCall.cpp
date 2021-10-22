#include "FIVSNode_UFunctionCall.h"
#include "FicsItNetworks/FicsItVisualScript/Editor/FIVSEdNodeViewer.h"
#include "UObject/PropertyIterator.h"

TMap<UClass*, TMap<FString, FFIVSNodeUFunctionCallMeta>> UFIVSNode_UFunctionCall::FunctionMetaData;

void UFIVSNode_UFunctionCall::InitPins() {
	for (TFieldIterator<FProperty> Prop(Function); Prop; ++Prop) {
		auto Flags = Prop->GetPropertyFlags();
		UFIVSPin* Pin = nullptr;
		EFIVSPinType PinType = FIVS_PIN_DATA;
		EFINNetworkValueType PinDataType = FIN_NIL;
		if (Flags & CPF_Parm) {
			if (Flags & CPF_OutParm) {
				PinType |= FIVS_PIN_OUTPUT;
			} else {
				PinType |= FIVS_PIN_INPUT;
			}
			
			if (Prop->IsA<FFloatProperty>()) {
				PinDataType = FIN_FLOAT;
			} else if (Prop->IsA<FIntProperty>() || Prop->IsA<FInt64Property>()) {
				PinDataType = FIN_INT;
			} else if (Prop->IsA<FBoolProperty>()) {
				PinDataType = FIN_BOOL;
			} else if (Prop->IsA<FStrProperty>()) {
				PinDataType = FIN_STR;
			}
			Pin = CreatePin(PinType, FText::FromString(Prop->GetName()), FFIVSPinDataType(PinDataType));
		}
		if (Pin) {
			PropertyToPin.Add(*Prop, Pin);
		}
	}
}

TArray<FFIVSNodeAction> UFIVSNode_UFunctionCall::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;
	for(TObjectIterator<UClass> It; It; ++It) {
		UClass* Class = *It;
		TMap<FString, FFIVSNodeUFunctionCallMeta>* FuncMeta = FunctionMetaData.Find(Class);
		for (TFieldIterator<UFunction> Func(Class); Func; ++Func) {
			FFIVSNodeUFunctionCallMeta* Meta = FuncMeta ? FuncMeta->Find(Func->GetName()) : nullptr;
			if (!Func->GetName().StartsWith("FIVSFunc_")) continue;
			FString MetaSymbol;
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_UFunctionCall::StaticClass();
			if (Meta) {
				Action.Title = Meta->Title;
				Action.Category = Meta->Categrory;
				Action.SearchableText = Meta->SearchableText;
				MetaSymbol = Meta->Symbol;
			} else {
				Action.Title = FText::FromString(Func->GetName());
				Action.Category = FText::FromString(TEXT("Math"));
				Action.SearchableText = Action.Title;
			}
			for (TFieldIterator<FProperty> Prop(*Func); Prop; ++Prop) {
				auto Flags = Prop->GetPropertyFlags();
				UFIVSPin* Pin = nullptr;
				EFIVSPinType PinType = FIVS_PIN_DATA;
				EFINNetworkValueType PinDataType = FIN_NIL;
				if (Flags & CPF_Parm) {
					if (Flags & CPF_OutParm) {
						PinType |= FIVS_PIN_OUTPUT;
					} else {
						PinType |= FIVS_PIN_INPUT;
					}
				
					if (Prop->IsA<FFloatProperty>()) {
						PinDataType = FIN_FLOAT;
					} else if (Prop->IsA<FIntProperty>() || Prop->IsA<FInt64Property>()) {
						PinDataType = FIN_INT;
					} else if (Prop->IsA<FBoolProperty>()) {
						PinDataType = FIN_BOOL;
					} else if (Prop->IsA<FStrProperty>()) {
						PinDataType = FIN_STR;
					}
					Action.Pins.Add(FFIVSFullPinType(PinType, FFIVSPinDataType(PinDataType)));
				}
			}
			UFunction* F = *Func;
			Action.OnExecute.BindLambda([F, MetaSymbol](UFIVSNode* Node) {
				Cast<UFIVSNode_UFunctionCall>(Node)->Function = F;
				Cast<UFIVSNode_UFunctionCall>(Node)->Symbol = MetaSymbol;
			});
			Actions.Add(Action);
		}
	}
	return Actions;
}

FString UFIVSNode_UFunctionCall::GetNodeName() const {
	return Function->GetName();
}

TSharedRef<SFIVSEdNodeViewer> UFIVSNode_UFunctionCall::CreateNodeViewer(SFIVSEdGraphViewer* GraphViewer, const FFIVSEdStyle* Style) {
	return SNew(SFIVSEdOperatorNodeViewer, GraphViewer, this)
	.Style(Style)
	.Symbol(Symbol);
}

void UFIVSNode_UFunctionCall::SerializeNodeProperties(FFIVSNodeProperties& Properties) const {
	Properties.Properties.Add(TEXT("Function"), Function->GetPathName());
	Properties.Properties.Add(TEXT("Symbol"), Symbol);
}

void UFIVSNode_UFunctionCall::DeserializeNodeProperties(const FFIVSNodeProperties& Properties) {
	Function = Cast<UFunction>(FSoftObjectPath(Properties.Properties[TEXT("Function")]).TryLoad());
	Symbol = Properties.Properties[TEXT("Symbol")];
}

TArray<UFIVSPin*> UFIVSNode_UFunctionCall::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<UFIVSPin*> InputPins;
	PropertyToPin.GenerateValueArray(InputPins);
	return InputPins.FilterByPredicate([](UFIVSPin* Pin) {
		return Pin->GetPinType() == FIVS_PIN_DATA_INPUT;
	});
}

UFIVSPin* UFIVSNode_UFunctionCall::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<FFINAnyNetworkValue> Output;
	
	// allocate & initialize parameter struct
	uint8* ParamStruct = (uint8*)FMemory_Alloca(Function->PropertiesSize);
	FFIVSValue Value;
	for (TFieldIterator<UProperty> Prop(Function); Prop; ++Prop) {
		if (Prop->GetPropertyFlags() & CPF_Parm) {
			if (Prop->IsInContainer(Function->ParmsSize)) {
				Prop->InitializeValue_InContainer(ParamStruct);
				if (!(Prop->GetPropertyFlags() & CPF_OutParm)) {
					Value = Context.GetValue(PropertyToPin[*Prop]);
					Value->Copy(*Prop, Prop->ContainerPtrToValuePtr<void>(ParamStruct));
				}
			}
		}
	}
	
	UObject* Obj = const_cast<UObject*>(GetDefault<UObject>(Function->GetOuterUClass()));
	Obj->ProcessEvent(Function, ParamStruct);

	// destroy parameter struct
	for (TFieldIterator<UProperty> Prop(Function); Prop; ++Prop) {
		EPropertyFlags Flags = Prop->GetPropertyFlags();
		if (Flags & CPF_Parm) {
			if (Flags & CPF_OutParm) {
				Context.SetValue(PropertyToPin[*Prop], FFINAnyNetworkValue(*Prop, Prop->ContainerPtrToValuePtr<void>(ParamStruct)));
			}
			if (Prop->IsInContainer(Function->ParmsSize)) Prop->DestroyValue_InContainer(ParamStruct);
		}
	}
			
	return nullptr;
}
