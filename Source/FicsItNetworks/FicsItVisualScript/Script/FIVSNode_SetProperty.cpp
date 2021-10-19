#include "FIVSNode_SetProperty.h"

#include "FicsItNetworks/Network/FINNetworkUtils.h"
#include "FicsItNetworks/Reflection/FINReflection.h"

TArray<FFIVSNodeAction> UFIVSNode_SetProperty::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;
	for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
		for (UFINProperty* SetProperty : Class.Value->GetProperties(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_SetProperty::StaticClass();
			Action.Title = FText::FromString(TEXT("Set ") + SetProperty->GetDisplayName().ToString());
			Action.SearchableText = Action.Title;
			Action.Category = FText::FromString(Class.Value->GetDisplayName().ToString() + TEXT("|Properties"));
			Action.Pins.Add(FIVS_PIN_EXEC_INPUT);
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(FIN_TRACE, Class.Value)));
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
	return Actions;
}

void UFIVSNode_SetProperty::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
	InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Instance"), FFIVSPinDataType(FIN_TRACE, Cast<UFINClass>(Property->GetOuter())));
	FFIVSPinDataType Type(Property);
	if (Type.GetType() == FIN_OBJ) Type = FFIVSPinDataType(FIN_TRACE, Type.GetRefSubType());
	DataIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Value"), Type);
}

TArray<UFIVSPin*> UFIVSNode_SetProperty::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{InstanceIn, DataIn};
}

UFIVSPin* UFIVSNode_SetProperty::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFINNetworkTrace Instance = Context.GetValue(InstanceIn).GetTrace();
	FFINAnyNetworkValue Data = Context.GetValue(DataIn);
	FFINExecutionContext ExecContext(UFINNetworkUtils::RedirectIfPossible(Instance));
	Property->SetValue(ExecContext, Data);
	return ExecOut;
}
