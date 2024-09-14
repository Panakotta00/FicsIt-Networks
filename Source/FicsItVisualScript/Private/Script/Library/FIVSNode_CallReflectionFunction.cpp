#include "Script/Library/FIVSNode_CallReflectionFunction.h"

#include "FIVSUtils.h"
#include "Kernel/FIVSRuntimeContext.h"
#include "Network/FINNetworkUtils.h"
#include "Reflection/FINReflection.h"

void FFIVSNodeStatement_CallReflectionFunction::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(InputPins);
}

void FFIVSNodeStatement_CallReflectionFunction::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	TArray<FFINAnyNetworkValue> InputValues;
	FFINExecutionContext ExecContext;
	if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) {
		ExecContext = Context.TryGetRValue(Self)->GetClass();
	} else {
		ExecContext = (UFINNetworkUtils::RedirectIfPossible(Context.TryGetRValue(Self)->GetTrace()));
	}

	for (FGuid inputPin : InputPins) {
		InputValues.Add(*Context.TryGetRValue(inputPin));
	}

	TArray<FFINAnyNetworkValue> OutputValues = Function->Execute(ExecContext, InputValues);

	for (int i = 0; i < FMath::Min(OutputValues.Num(), OutputPins.Num()); ++i) {
		Context.SetValue(OutputPins[i], OutputValues[i]);
	}

	Context.Push_ExecPin(ExecOut);
}

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

void UFIVSNode_CallReflectionFunction::SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const {
	Value->SetStringField(TEXT("function"), Function->GetPathName());
}

void UFIVSNode_CallReflectionFunction::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) {
	if (!Value) return;

	FString functionStr;
	if (Value->TryGetStringField(TEXT("function"), functionStr)) {
		SetFunction(Cast<UFINFunction>(FSoftObjectPath(functionStr).TryLoad()));
	}
}

TFINDynamicStruct<FFIVSNodeStatement> UFIVSNode_CallReflectionFunction::CreateNodeStatement() {
	return FFIVSNodeStatement_CallReflectionFunction{
		NodeId,
		ExecIn->PinId,
		ExecOut->PinId,
		Self->PinId,
		UFIVSUtils::GuidsFromPins(InputPins),
		UFIVSUtils::GuidsFromPins(OutputPins),
		Function,
	};
}

void UFIVSNode_CallReflectionFunction::SetFunction(UFINFunction* InFunction) {
	if (InFunction == nullptr) return;

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
