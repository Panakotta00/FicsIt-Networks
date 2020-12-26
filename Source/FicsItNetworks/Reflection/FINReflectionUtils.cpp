#include "FINReflectionUtils.h"

#include "FINArrayProperty.h"

bool UFINReflectionUtils::CheckIfVarargs(UFINProperty* Prop) {
	if (Prop->GetInternalName() != "varargs") return false;
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Prop);
	if (!ArrayProperty || !ArrayProperty->Inner) return false;
	UFINStructProperty* StructProperty = Cast<UFINStructProperty>(ArrayProperty->Inner);
	if (!StructProperty || StructProperty->Struct != FFINAnyNetworkValue::StaticStruct()) return false;
	return true;
}

bool UFINReflectionUtils::SetIfVarargs(UFINProperty* Prop, const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params) {
	if (!CheckIfVarargs(Prop)) return false;
	UFINArrayProperty* ArrProp = Cast<UFINArrayProperty>(Prop);
	ArrProp->SetValue(Ctx, Params);
	return true;
}
