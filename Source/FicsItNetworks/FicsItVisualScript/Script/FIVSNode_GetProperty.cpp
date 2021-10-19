#include "FIVSNode_GetProperty.h"

#include "FicsItNetworks/Reflection/FINReflection.h"

TArray<FFIVSNodeAction> UFIVSNode_GetProperty::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;
	for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
		for (UFINProperty* GetProperty : Class.Value->GetProperties(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_GetProperty::StaticClass();
			Action.Title = FText::FromString(TEXT("Get ") + GetProperty->GetDisplayName().ToString());
			Action.SearchableText = Action.Title;
			Action.Category = FText::FromString(Class.Value->GetDisplayName().ToString() + TEXT("|Properties"));
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(FIN_TRACE, Class.Value)));
			FFIVSFullPinType PinType(GetProperty);
			PinType.PinType |= FIVS_PIN_OUTPUT;
			Action.Pins.Add(PinType);
			Action.OnExecute.BindLambda([GetProperty](UFIVSNode* Node) {
				Cast<UFIVSNode_GetProperty>(Node)->SetProperty(GetProperty);
			});
			Actions.Add(Action);
		}
	}
	return Actions;
}

void UFIVSNode_GetProperty::InitPins() {
	InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Instance"), {FIN_TRACE, Cast<UFINClass>(Property->GetOuter())});
	FFIVSPinDataType Type(Property);
	if (Type.GetType() == FIN_OBJ) Type = FFIVSPinDataType(FIN_TRACE, Type.GetRefSubType());
	DataOut = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString("Value"), Type);
}

TArray<UFIVSPin*> UFIVSNode_GetProperty::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{InstanceIn};
}

UFIVSPin* UFIVSNode_GetProperty::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFINNetworkTrace Instance = Context.GetValue(InstanceIn).GetTrace();
	FFINExecutionContext ExecContext(UFINNetworkUtils::RedirectIfPossible(Instance));
	FFINAnyNetworkValue Value = Property->GetValue(ExecContext);
	Context.SetValue(DataOut, Value);
	return nullptr;
}
