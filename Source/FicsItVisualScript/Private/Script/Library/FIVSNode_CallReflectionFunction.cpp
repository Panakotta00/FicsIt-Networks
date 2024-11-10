#include "Script/Library/FIVSNode_CallReflectionFunction.h"

#include "FicsItReflection.h"
#include "FINNetworkUtils.h"
#include "FIRClass.h"
#include "FIVSUtils.h"
#include "JsonObject.h"
#include "Kernel/FIVSRuntimeContext.h"

UFIVSNode_CallReflectionFunction::UFIVSNode_CallReflectionFunction() {
	ExecIn = CreateDefaultPin(FIVS_PIN_EXEC_INPUT, TEXT("Exec"), FText::FromString(TEXT("Exec")));
	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Run"), FText::FromString(TEXT("Run")));
}

void UFIVSNode_CallReflectionFunction::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (TPair<UClass*, UFIRClass*> Class : FFicsItReflectionModule::Get().GetClasses()) {
		for (UFIRFunction* CallFunction : Class.Value->GetFunctions(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_CallReflectionFunction::StaticClass();
			if (CallFunction->GetFunctionFlags() & FIR_Func_ClassFunc) {
				Action.Title = FText::FromString(CallFunction->GetDisplayName().ToString() + TEXT(" (Class)"));
			} else {
				Action.Title = CallFunction->GetDisplayName();
			}
			Action.Category = FText::FromString(Class.Value->GetDisplayName().ToString() + TEXT("|Functions"));
			Action.SearchableText = CallFunction->GetDisplayName();
			Action.Pins.Add(FIVS_PIN_EXEC_INPUT);
			Action.Pins.Add(FIVS_PIN_EXEC_OUTPUT);
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIRExtendedValueType(CallFunction->GetFunctionFlags() & FIR_Func_ClassFunc ? FIR_CLASS : FIR_TRACE, Class.Value)));
			for (UFIRProperty* Param : CallFunction->GetParameters()) {
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
		SetFunction(Cast<UFIRFunction>(FSoftObjectPath(functionStr).TryLoad()));
	}
}

void UFIVSNode_CallReflectionFunction::CompileNodeToLua(FFIVSLuaCompilerContext& Context) const {
	FString self = Context.GetRValueExpression(Self);
	FString functionName = Function->GetInternalName();
	TArray<FString> parameters;
	for (UFIVSPin* pin : InputPins) {
		parameters.Add(Context.GetRValueExpression(pin));
	}
	TArray<FString> returnValues;
	for (UFIVSPin* pin : OutputPins) {
		FString name = Context.FindAndClaimLocalName(pin->GetName());
		returnValues.Add(name);
		Context.AddLValue(pin, name);
	}

	if (returnValues.IsEmpty()) {
		Context.AddPlain(FString::Printf(TEXT("%s:%s(%s)\n"), *self, *functionName, *FString::Join(parameters, TEXT(","))));
	} else {
		Context.AddPlain(FString::Printf(TEXT("local %s = %s:%s(%s)\n"), *FString::Join(returnValues, TEXT(",")), *self, *functionName, *FString::Join(parameters, TEXT(","))));
	}

	Context.ContinueCurrentSection(ExecOut);
}

void UFIVSNode_CallReflectionFunction::SetFunction(UFIRFunction* InFunction) {
	if (InFunction == nullptr) return;

	Function = InFunction;

	if (Function->GetFunctionFlags() & FIR_Func_ClassFunc) {
		DisplayName = FText::FromString(Function->GetDisplayName().ToString() + TEXT(" (Class)"));
	} else {
		DisplayName = FText::FromString(Function->GetDisplayName().ToString());
	}

	DeletePin(Self);
	DeletePins(OutputPins);
	DeletePins(InputPins);

	Self = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("self"), FText::FromString(TEXT("Self")), FFIVSPinDataType(Function->GetFunctionFlags() & FIR_Func_ClassFunc ? FIR_CLASS : FIR_TRACE, Cast<UFIRClass>(Function->GetOuter())));
	for (UFIRProperty* Param : Function->GetParameters()) {
		EFIRPropertyFlags Flags = Param->GetPropertyFlags();
		if (Flags & FIR_Prop_Param) {
			FFIVSPinDataType Type = FFIRExtendedValueType(Param);
			if (Type.GetType() == FIR_OBJ) Type = FFIVSPinDataType(FIR_TRACE, Type.GetRefSubType());
			if (Flags & FIR_Prop_OutParam) {
				OutputPins.Add(CreatePin(FIVS_PIN_DATA_OUTPUT, Param->GetInternalName(), Param->GetDisplayName(), Type));
			} else {
				InputPins.Add(CreatePin(FIVS_PIN_DATA_INPUT, Param->GetInternalName(), Param->GetDisplayName(), Type));
			}
		}
	}
}
