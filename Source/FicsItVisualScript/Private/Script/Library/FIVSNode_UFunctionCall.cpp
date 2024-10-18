#include "Script/Library/FIVSNode_UFunctionCall.h"

#include "Editor/FIVSEdNodeViewer.h"
#include "Kernel/FIVSRuntimeContext.h"

TMap<UClass*, TMap<FString, FFIVSNodeUFunctionCallMeta>> UFIVSNode_UFunctionCall::FunctionMetaData;

void FFIVSNodeStatement_UFunctionCall::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(InputPins);
}

void FFIVSNodeStatement_UFunctionCall::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	TArray<FFIRAnyValue> Output;

	// allocate & initialize parameter struct
	uint8* ParamStruct = (uint8*)FMemory_Alloca(Function->PropertiesSize);
	for (TFieldIterator<FProperty> Prop(Function); Prop; ++Prop) {
		if (Prop->GetPropertyFlags() & CPF_Parm) {
			if (Prop->IsInContainer(Function->ParmsSize)) {
				Prop->InitializeValue_InContainer(ParamStruct);
				if (!(Prop->GetPropertyFlags() & CPF_OutParm)) {
					const FFIRAnyValue* Value = Context.TryGetRValue(PropertyToPin[Prop->GetFName()]);
					Value->CopyToProperty(*Prop, Prop->ContainerPtrToValuePtr<void>(ParamStruct));
				}
			}
		}
	}

	UObject* Obj = const_cast<UObject*>(GetDefault<UObject>(Function->GetOuterUClass()));
	Obj->ProcessEvent(Function, ParamStruct);

	// destroy parameter struct
	for (TFieldIterator<FProperty> Prop(Function); Prop; ++Prop) {
		EPropertyFlags Flags = Prop->GetPropertyFlags();
		if (Flags & CPF_Parm) {
			if (Flags & CPF_OutParm) {
				Context.SetValue(PropertyToPin[Prop->GetFName()], FFIRAnyValue::FromProperty(*Prop, Prop->ContainerPtrToValuePtr<void>(ParamStruct)));
			}
			if (Prop->IsInContainer(Function->ParmsSize)) Prop->DestroyValue_InContainer(ParamStruct);
		}
	}
}

void UFIVSNode_UFunctionCall::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
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
				EFIVSPinType PinType = FIVS_PIN_DATA;
				EFIRValueType PinDataType = FIR_NIL;
				if (Flags & CPF_Parm) {
					if (Flags & CPF_OutParm) {
						PinType |= FIVS_PIN_OUTPUT;
					} else {
						PinType |= FIVS_PIN_INPUT;
					}
				
					if (Prop->IsA<FFloatProperty>()) {
						PinDataType = FIR_FLOAT;
					} else if (Prop->IsA<FIntProperty>() || Prop->IsA<FInt64Property>()) {
						PinDataType = FIR_INT;
					} else if (Prop->IsA<FBoolProperty>()) {
						PinDataType = FIR_BOOL;
					} else if (Prop->IsA<FStrProperty>()) {
						PinDataType = FIR_STR;
					}
					Action.Pins.Add(FFIVSFullPinType(PinType, FFIVSPinDataType(PinDataType)));
				}
			}
			UFunction* F = *Func;
			Action.OnExecute.BindLambda([F, MetaSymbol](UFIVSNode* Node) {
				Cast<UFIVSNode_UFunctionCall>(Node)->SetFunction(F, MetaSymbol);
			});
			Actions.Add(Action);
		}
	}
}

TSharedRef<SFIVSEdNodeViewer> UFIVSNode_UFunctionCall::CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style) {
	return SNew(SFIVSEdOperatorNodeViewer, GraphViewer, this)
	.Style(Style)
	.Symbol(Symbol);
}

void UFIVSNode_UFunctionCall::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetStringField(TEXT("Function"), Function->GetPathName());
	Properties->SetStringField(TEXT("Symbol"), Symbol);
}

void UFIVSNode_UFunctionCall::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	SetFunction(Cast<UFunction>(FSoftObjectPath(Properties->GetStringField(TEXT("Function"))).TryLoad()), Properties->GetStringField(TEXT("Symbol")));
}

void UFIVSNode_UFunctionCall::SetFunction(UFunction* InFunction, const FString& InSymbol) {
	Function = InFunction;
	Symbol = InSymbol;

	DisplayName = FText::FromString(Function->GetName());

	DeletePin(ExecIn);
	DeletePin(ExecOut);
	DeletePins(Input);
	DeletePins(Output);

	for (TFieldIterator<FProperty> Prop(Function); Prop; ++Prop) {
		auto Flags = Prop->GetPropertyFlags();
		UFIVSPin* Pin = nullptr;
		EFIVSPinType PinType = FIVS_PIN_DATA;
		EFIRValueType PinDataType = FIR_NIL;
		if (Flags & CPF_Parm) {
			if (Flags & CPF_OutParm) {
				PinType |= FIVS_PIN_OUTPUT;
			} else {
				PinType |= FIVS_PIN_INPUT;
			}

			if (Prop->IsA<FFloatProperty>()) {
				PinDataType = FIR_FLOAT;
			} else if (Prop->IsA<FIntProperty>() || Prop->IsA<FInt64Property>()) {
				PinDataType = FIR_INT;
			} else if (Prop->IsA<FBoolProperty>()) {
				PinDataType = FIR_BOOL;
			} else if (Prop->IsA<FStrProperty>()) {
				PinDataType = FIR_STR;
			}
			Pin = CreatePin(PinType, Prop->GetName(), FText::FromString(Prop->GetName()), FFIVSPinDataType(PinDataType));
		}
		if (Pin) {
			PropertyToPin.Add(*Prop, Pin);
			if (Flags & CPF_OutParm) {
				Output.Add(Pin);
			} else {
				Input.Add(Pin);
			}
		}
	}
}
