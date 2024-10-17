#include "Script/Library/FIVSNode_SetProperty.h"

#include "FicsItReflection.h"
#include "FINNetworkUtils.h"
#include "Kernel/FIVSRuntimeContext.h"

void FFIVSNodeStatement_SetProperty::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(DataIn);
	Context.Push_EvaluatePin(InstanceIn);
}

void FFIVSNodeStatement_SetProperty::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	const FFIRAnyValue* Instance = Context.TryGetRValue(InstanceIn);
	const FFIRAnyValue* Data = Context.TryGetRValue(DataIn);
	FFIRExecutionContext ExecContext;
	if (Property->GetPropertyFlags() & FIR_Prop_ClassProp) {
		ExecContext = Instance->GetClass();
	} else {
		ExecContext = UFINNetworkUtils::RedirectIfPossible(Instance->GetTrace());
	}
	Property->SetValue(ExecContext, *Data);
	Context.Push_ExecPin(ExecOut);
}

void UFIVSNode_SetProperty::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (TPair<UClass*, UFIRClass*> Class : FFicsItReflectionModule::Get().GetClasses()) {
		for (UFIRProperty* SetProperty : Class.Value->GetProperties(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_SetProperty::StaticClass();
			if (SetProperty->GetPropertyFlags() & FIR_Prop_ClassProp) {
				Action.Title = FText::FromString(TEXT("Set ") + SetProperty->GetDisplayName().ToString() + TEXT(" (Class)"));
			} else {
				Action.Title = FText::FromString(TEXT("Set ") + SetProperty->GetDisplayName().ToString());
			}
			Action.SearchableText = Action.Title;
			Action.Category = FText::FromString(Class.Value->GetDisplayName().ToString() + TEXT("|Properties"));
			Action.Pins.Add(FIVS_PIN_EXEC_INPUT);
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIRExtendedValueType(SetProperty->GetPropertyFlags() & FIR_Prop_ClassProp ? FIR_CLASS : FIR_TRACE, Class.Value)));
			FFIVSFullPinType PinType(SetProperty);
			PinType.PinType |= FIVS_PIN_INPUT;
			Action.Pins.Add(PinType);
			Action.Pins.Add(FIVS_PIN_EXEC_OUTPUT);
			Action.OnExecute.BindLambda([SetProperty](UFIVSNode* Node) {
				Cast<UFIVSNode_SetProperty>(Node)->SetProperty(SetProperty);
			});
			Actions.Add(Action);
		}
	}
}

void UFIVSNode_SetProperty::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetStringField(TEXT("Property"), Property->GetPathName());
}

void UFIVSNode_SetProperty::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	SetProperty(Cast<UFIRProperty>(FSoftObjectPath(Properties->GetStringField(TEXT("Property"))).TryLoad()));
}

void UFIVSNode_SetProperty::SetProperty(UFIRProperty* InProperty) {
	Property = InProperty;

	if (Property->GetPropertyFlags() & FIR_Prop_ClassProp) {
		DisplayName = FText::FromString(TEXT("Set ") + Property->GetInternalName() + TEXT(" (Class)"));
	} else {
		DisplayName = FText::FromString(TEXT("Set ") + Property->GetInternalName());
	}

	DeletePin(ExecIn);
	DeletePin(ExecOut);
	DeletePin(InstanceIn);
	DeletePin(DataIn);

	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString("Exec"));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, TEXT("Out"), FText::FromString("Out"));
	InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Instance"), FText::FromString("Instance"), FFIVSPinDataType(Property->GetPropertyFlags() & FIR_Prop_ClassProp ? FIR_CLASS : FIR_TRACE, Cast<UFIRClass>(Property->GetOuter())));
	FFIVSPinDataType Type(Property);
	if (Type.GetType() == FIR_OBJ) Type = FFIVSPinDataType(FIR_TRACE, Type.GetRefSubType());
	DataIn = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Value"), FText::FromString("Value"), Type);
}
