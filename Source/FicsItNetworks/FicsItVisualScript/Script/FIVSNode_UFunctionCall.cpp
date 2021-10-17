#include "FIVSNode_UFunctionCall.h"
#include "FicsItNetworks/FicsItVisualScript/Editor/FIVSEdNodeViewer.h"
#include "FIVSMathLib.h"
#include "UObject/PropertyIterator.h"

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
			} else if (Prop->IsA<FIntProperty>()) {
				PinDataType = FIN_INT;
			} else if (Prop->IsA<FBoolProperty>()) {
				PinDataType = FIN_BOOL;
			}
			Pin = CreatePin(PinType, FText::FromString(Prop->GetName()), PinDataType);
		}
		if (Pin) {
			PropertyToPin.Add(*Prop, Pin);
		}
	}
}

TArray<FFIVSNodeAction> UFIVSNode_UFunctionCall::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;
	for (TFieldIterator<UFunction> Func(UFIVSMathLib::StaticClass()); Func; ++Func) {
		if (!Func->GetName().StartsWith("FIVSFunc_")) continue;
		FFIVSNodeAction Action;
		Action.NodeType = UFIVSNode_UFunctionCall::StaticClass();
		Action.Title = FText::FromString(Func->GetName());
		Action.Category = FText::FromString(TEXT("Math"));
		Action.SearchableText = Action.Title;
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
				} else if (Prop->IsA<FIntProperty>()) {
					PinDataType = FIN_INT;
				} else if (Prop->IsA<FBoolProperty>()) {
					PinDataType = FIN_BOOL;
				}
				Action.Pins.Add(FFIVSFullPinType(PinType, PinDataType));
			}
		}
		UFunction* F = *Func;
		Action.OnExecute.BindLambda([F](UFIVSNode* Node) {
			Cast<UFIVSNode_UFunctionCall>(Node)->Function = F;
		});
		Actions.Add(Action);
	}
	return Actions;
}

FString UFIVSNode_UFunctionCall::GetNodeName() const {
	return Function->GetName();
}

TSharedRef<SFIVSEdNodeViewer> UFIVSNode_UFunctionCall::CreateNodeViewer(SFIVSEdGraphViewer* GraphViewer, const FFIVSEdStyle* Style) const {
	return SNew(SFIVSEdOperatorNodeViewer, GraphViewer, this)
	.Style(Style);
}

TArray<UFIVSPin*> UFIVSNode_UFunctionCall::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<UFIVSPin*> InputPins;
	PropertyToPin.GenerateValueArray(InputPins);
	return InputPins.FilterByPredicate([](UFIVSPin* Pin) {
		return Pin->GetPinType() == FIVS_PIN_DATA_INPUT;
	});
}
#pragma optimize("", off)
UFIVSPin* UFIVSNode_UFunctionCall::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<FFINAnyNetworkValue> Output;
	// allocate & initialize parameter struct
	uint8* ParamStruct = (uint8*)FMemory_Alloca(Function->PropertiesSize);
	for (TFieldIterator<UProperty> Prop(Function); Prop; ++Prop) {
		if (Prop->GetPropertyFlags() & CPF_Parm) {
			if (Prop->IsInContainer(Function->ParmsSize)) {
				Prop->InitializeValue_InContainer(ParamStruct);
				if (!(Prop->GetPropertyFlags() & CPF_OutParm)) {
					//if (Params.Num() <= i) throw FFINReflectionException(const_cast<UFINUFunction*>(this), FString::Printf(TEXT("Required parameter '%s' is not provided."), *Param->GetInternalName()));
					FFINAnyNetworkValue Value = Context.GetValue(PropertyToPin[*Prop]);
					Value.Copy(*Prop, Prop->ContainerPtrToValuePtr<void>(ParamStruct));
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
#pragma optimize("", on)
