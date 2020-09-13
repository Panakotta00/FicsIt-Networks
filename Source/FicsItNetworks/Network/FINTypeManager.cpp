#include "FINTypeManager.h"

#include "FINNetworkCustomType.h"

FFINFunctionParameter::FFINFunctionParameter(UProperty* Prop) {
	Name = Prop->GetName();
	Type = GetValueTypeFromProp(Prop);
	bOutputValue = Prop->PropertyFlags & (CPF_OutParm | CPF_ReturnParm);
}

FString FFINUFunction::GetName() {
	return Func->GetName().RightChop(FString("netFunc_").Len());
}

TArray<FFINFunctionParameter> FFINUFunction::GetParameters() {
	TArray<FFINFunctionParameter> Params;
	for (TFieldIterator<UProperty> Prop(Func); Prop; ++Prop) {
		if (Prop->PropertyFlags | CPF_Parm) Params.Add(FFINFunctionParameter(*Prop));
	}
	return Params;
}

FString FFINUFunction::GetDescription() {
	return "";
}

UFunction* FFINUFunction::GetUFunc() const {
	return Func;
}

bool FFINUFunction::IsNetworkFunction(UFunction* Func) {
	return Func->GetName().StartsWith("netFunc_");
}

FString FFINUType::GetName() {
	if (Type->ImplementsInterface(UFINNetworkCustomType::StaticClass())) {
		return IFINNetworkCustomType::Execute_GetCustomTypeName(Type->GetDefaultObject());
	}
	return Type->GetDisplayNameText().ToString();
}

TArray<TSharedRef<FFINFunction>> FFINUType::GetFunctions() {
	TArray<TSharedRef<FFINFunction>> Functions;
	for (TFieldIterator<UFunction> Func(Type); Func; ++Func) {
		if (Func->GetOwnerClass() != Type || !FFINUFunction::IsNetworkFunction(*Func)) continue;
		Functions.Add(MakeShared<FFINUFunction>(*Func));
	}
	return Functions;
}
