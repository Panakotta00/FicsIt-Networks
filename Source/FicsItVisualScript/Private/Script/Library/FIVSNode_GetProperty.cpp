#include "Script/Library/FIVSNode_GetProperty.h"

#include "Network/FINNetworkUtils.h"
#include "Reflection/FINReflection.h"

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

void UFIVSNode_GetProperty::SerializeNodeProperties(FFIVSNodeProperties& Properties) const {
	Properties.Properties.Add(TEXT("Property"), Property->GetPathName());
}

void UFIVSNode_GetProperty::DeserializeNodeProperties(const FFIVSNodeProperties& Properties) {
	Property = Cast<UFINProperty>(FSoftObjectPath(Properties.Properties[TEXT("Property")]).TryLoad());
}

TArray<UFIVSPin*> UFIVSNode_GetProperty::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{InstanceIn};
}

UFIVSPin* UFIVSNode_GetProperty::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFIVSValue Instance = Context.GetValue(InstanceIn);
	FFINExecutionContext ExecContext;
	if (Property->GetPropertyFlags() & FIN_Prop_ClassProp) ExecContext = Instance->GetClass();
	else ExecContext = UFINNetworkUtils::RedirectIfPossible(Instance->GetTrace());
	FFINAnyNetworkValue Value = Property->GetValue(ExecContext);
	Context.SetValue(DataOut, Value);
	return nullptr;
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
