#include "FIVSNode_CallReflectionFunction.h"

#include "FicsItNetworks/Network/FINNetworkUtils.h"
#include "FicsItNetworks/Reflection/FINReflection.h"

TArray<FFIVSNodeAction> UFIVSNode_CallReflectionFunction::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;
	for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
		for (UFINFunction* CallFunction : Class.Value->GetFunctions(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_CallReflectionFunction::StaticClass();
			Action.Title = CallFunction->GetDisplayName();
			Action.Category = FText::FromString(Class.Value->GetDisplayName().ToString() + TEXT("|Functions"));
			Action.SearchableText = CallFunction->GetDisplayName();
			Action.Pins.Add(FIVS_PIN_EXEC_INPUT);
			Action.Pins.Add(FIVS_PIN_EXEC_OUTPUT);
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(FIN_TRACE, Class.Value)));
			for (UFINProperty* Param : CallFunction->GetParameters()) {
				Action.Pins.Add(FFIVSFullPinType(Param));
			}
			Action.OnExecute.BindLambda([CallFunction](UFIVSNode* Node) {
				Cast<UFIVSNode_CallReflectionFunction>(Node)->SetFunction(CallFunction);
			});
			Actions.Add(Action);
		}
	}
	return Actions;
}

void UFIVSNode_CallReflectionFunction::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString(TEXT("Exec")));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString(TEXT("Run")));
	Self = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("Self")), {FIN_TRACE, Cast<UFINClass>(Function->GetOuter())});
	for (UFINProperty* Param : Function->GetParameters()) {
		EFINRepPropertyFlags Flags = Param->GetPropertyFlags();
		if (Flags & FIN_Prop_Param) {
			FFIVSPinDataType Type = Param;
			if (Type.GetType() == FIN_OBJ) Type = FFIVSPinDataType(FIN_TRACE, Type.GetRefSubType());
			if (Flags & FIN_Prop_OutParam) {
				OutputPins.Add(CreatePin(FIVS_PIN_DATA_OUTPUT, Param->GetDisplayName(), Type));
			} else {
				InputPins.Add(CreatePin(FIVS_PIN_DATA_INPUT, Param->GetDisplayName(), Type));
			}
		}
	}
}

TArray<UFIVSPin*> UFIVSNode_CallReflectionFunction::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<UFIVSPin*> EvalPins = InputPins;
	EvalPins.Add(Self);
	return EvalPins;
}

UFIVSPin* UFIVSNode_CallReflectionFunction::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<FFINAnyNetworkValue> InputValues;
	FFINExecutionContext ExecContext(UFINNetworkUtils::RedirectIfPossible(Context.GetValue(Self).GetTrace()));
	for (UFIVSPin* InputPin : InputPins) {
		InputValues.Add(Context.GetValue(InputPin));
	}
	TArray<FFINAnyNetworkValue> OutputValues = Function->Execute(ExecContext, InputValues);
	for (int i = 0; i < FMath::Min(OutputValues.Num(), OutputPins.Num()); ++i) {
		Context.SetValue(OutputPins[i], OutputValues[i]);
	}
	return ExecOut;
}
