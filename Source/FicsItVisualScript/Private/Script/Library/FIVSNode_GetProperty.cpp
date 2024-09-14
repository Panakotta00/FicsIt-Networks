#include "Script/Library/FIVSNode_GetProperty.h"

#include "Kernel/FIVSRuntimeContext.h"
#include "Network/FINNetworkUtils.h"
#include "Reflection/FINReflection.h"

void FFIVSNodeStatement_GetProperty::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(DataIn);
}

void FFIVSNodeStatement_GetProperty::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	const FFINAnyNetworkValue* Instance = Context.TryGetRValue(DataIn);

	if (!Instance) return;

	FFINExecutionContext ExecContext;
	if (Property->GetPropertyFlags() & FIN_Prop_ClassProp) ExecContext = Instance->GetClass();
	else ExecContext = UFINNetworkUtils::RedirectIfPossible(Instance->GetTrace());

	FFINAnyNetworkValue Value = Property->GetValue(ExecContext);

	Context.SetValue(DataOut, Value);
}

void UFIVSNode_GetProperty::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
		for (UFINProperty* GetProperty : Class.Value->GetProperties(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_GetProperty::StaticClass();
			if (GetProperty->GetPropertyFlags() & FIN_Prop_ClassProp) {
				Action.Title = FText::FromString(TEXT("Get ") + GetProperty->GetDisplayName().ToString() + TEXT(" (Class)"));
			} else {
				Action.Title = FText::FromString(TEXT("Get ") + GetProperty->GetDisplayName().ToString());
			}
			Action.SearchableText = Action.Title;
			Action.Category = FText::FromString(Class.Value->GetDisplayName().ToString() + TEXT("|Properties"));
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(GetProperty->GetPropertyFlags() & FIN_Prop_ClassProp ? FIN_CLASS : FIN_TRACE, Class.Value)));
			FFIVSFullPinType PinType(GetProperty);
			PinType.PinType |= FIVS_PIN_OUTPUT;
			Action.Pins.Add(PinType);
			Action.OnExecute.BindLambda([GetProperty](UFIVSNode* Node) {
				Cast<UFIVSNode_GetProperty>(Node)->SetProperty(GetProperty);
			});
			Actions.Add(Action);
		}
	}
}

void UFIVSNode_GetProperty::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetStringField(TEXT("Property"), Property->GetPathName());
}

void UFIVSNode_GetProperty::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	SetProperty(Cast<UFINProperty>(FSoftObjectPath(Properties->GetStringField(TEXT("Property"))).TryLoad()));
}

void UFIVSNode_GetProperty::SetProperty(UFINProperty* InProperty) {
	Property = InProperty;

	if (Property->GetPropertyFlags() & FIN_Prop_ClassProp) {
		DisplayName = FText::FromString(TEXT("Get ") + Property->GetInternalName() + TEXT(" (Class)"));
	} else {
		DisplayName = FText::FromString(TEXT("Get ") + Property->GetInternalName());
	}

	DeletePin(InstanceIn);
	DeletePin(DataOut);

	InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Instance"), FText::FromString("Instance"), FFIVSPinDataType(Property->GetPropertyFlags() & FIN_Prop_ClassProp ? FIN_CLASS : FIN_TRACE, Cast<UFINClass>(Property->GetOuter())));
	FFIVSPinDataType Type(Property);
	if (Type.GetType() == FIN_OBJ) Type = FFIVSPinDataType(FIN_TRACE, Type.GetRefSubType());
	DataOut = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Value"), FText::FromString("Value"), Type);
}
