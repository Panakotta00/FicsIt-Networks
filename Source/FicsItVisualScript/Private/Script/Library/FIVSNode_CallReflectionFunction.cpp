#include "Script/Library/FIVSNode_CallReflectionFunction.h"

#include "Network/FINNetworkUtils.h"
#include "Reflection/FINReflection.h"

UFIVSNode_CallReflectionFunction::UFIVSNode_CallReflectionFunction() {
	ExecIn = CreateDefaultPin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString(TEXT("Exec")));
	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Run"), FText::FromString(TEXT("Run")));
}

void UFIVSNode_CallReflectionFunction::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
		for (UFINFunction* CallFunction : Class.Value->GetFunctions(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_CallReflectionFunction::StaticClass();
			if (CallFunction->GetFunctionFlags() & FIN_Func_ClassFunc) {
				Action.Title = FText::FromString(CallFunction->GetDisplayName().ToString() + TEXT(" (Class)"));
			} else {
				Action.Title = CallFunction->GetDisplayName();
			}
			Action.Category = FText::FromString(Class.Value->GetDisplayName().ToString() + TEXT("|Functions"));
			Action.SearchableText = CallFunction->GetDisplayName();
			Action.Pins.Add(FIVS_PIN_EXEC_INPUT);
			Action.Pins.Add(FIVS_PIN_EXEC_OUTPUT);
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(CallFunction->GetFunctionFlags() & FIN_Func_ClassFunc ? FIN_CLASS : FIN_TRACE, Class.Value)));
			for (UFINProperty* Param : CallFunction->GetParameters()) {
				Action.Pins.Add(FFIVSFullPinType(Param));
			}
			Action.OnExecute.BindLambda([CallFunction](UFIVSNode* Node) {
				Cast<UFIVSNode_CallReflectionFunction>(Node)->SetFunction(CallFunction);
			});
			Actions.Add(Action);
		}
	}
}

void UFIVSNode_CallReflectionFunction::SerializeNodeProperties(FFIVSNodeProperties& Properties) const {
	Properties.Properties.Add(TEXT("Function"), Function->GetPathName());
}

void UFIVSNode_CallReflectionFunction::DeserializeNodeProperties(const FFIVSNodeProperties& Properties) {
	SetFunction(Cast<UFINFunction>(FSoftObjectPath(Properties.Properties["Function"]).TryLoad()));
}

TArray<UFIVSPin*> UFIVSNode_CallReflectionFunction::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<UFIVSPin*> EvalPins = InputPins;
	EvalPins.Add(Self);
	return EvalPins;
}

UFIVSPin* UFIVSNode_CallReflectionFunction::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<FFINAnyNetworkValue> InputValues;
	FFINExecutionContext ExecContext;
	if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) ExecContext = Context.GetValue(Self)->GetClass();
	else ExecContext = (UFINNetworkUtils::RedirectIfPossible(Context.GetValue(Self)->GetTrace()));
	for (UFIVSPin* InputPin : InputPins) {
		InputValues.Add(*Context.GetValue(InputPin));
	}
	TArray<FFINAnyNetworkValue> OutputValues = Function->Execute(ExecContext, InputValues);
	for (int i = 0; i < FMath::Min(OutputValues.Num(), OutputPins.Num()); ++i) {
		Context.SetValue(OutputPins[i], OutputValues[i]);
	}
	return ExecOut;
}

void UFIVSNode_CallReflectionFunction::SetFunction(UFINFunction* InFunction) {
	check(ExecIn == nullptr);
	Function = InFunction;

	if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) {
		DisplayName = FText::FromString(Function->GetDisplayName().ToString() + TEXT(" (Class)"));
	} else {
		DisplayName = FText::FromString(Function->GetDisplayName().ToString());
	}

	DeletePin(Self);
	DeletePins(OutputPins);
	DeletePins(InputPins);

	Self = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Run"), FText::FromString(TEXT("Self")), FFIVSPinDataType(Function->GetFunctionFlags() & FIN_Func_ClassFunc ? FIN_CLASS : FIN_TRACE, Cast<UFINClass>(Function->GetOuter())));
	for (UFINProperty* Param : Function->GetParameters()) {
		EFINRepPropertyFlags Flags = Param->GetPropertyFlags();
		if (Flags & FIN_Prop_Param) {
			FFIVSPinDataType Type = FFINExpandedNetworkValueType(Param);
			if (Type.GetType() == FIN_OBJ) Type = FFIVSPinDataType(FIN_TRACE, Type.GetRefSubType());
			if (Flags & FIN_Prop_OutParam) {
				OutputPins.Add(CreatePin(FIVS_PIN_DATA_OUTPUT, Param->GetInternalName(), Param->GetDisplayName(), Type));
			} else {
				InputPins.Add(CreatePin(FIVS_PIN_DATA_INPUT, Param->GetInternalName(), Param->GetDisplayName(), Type));
			}
		}
	}
}
