#include "FINReflectionUtils.h"

#include "FINArrayProperty.h"

bool UFINReflectionUtils::CheckIfVarargs(UFINProperty* Prop) {
	if (!Prop->GetInternalName().Equals("varargs")) return false;
	UFINArrayProperty* ArrayProperty = Cast<UFINArrayProperty>(Prop);
	if (!ArrayProperty || !ArrayProperty->InnerType) return false;
	UFINStructProperty* StructProperty = Cast<UFINStructProperty>(ArrayProperty->InnerType);
	if (!StructProperty || StructProperty->Struct != FFINAnyNetworkValue::StaticStruct()) return false;
	return true;
}

bool UFINReflectionUtils::SetIfVarargs(UFINProperty* Prop, const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params) {
	if (!CheckIfVarargs(Prop)) return false;
	UFINArrayProperty* ArrProp = Cast<UFINArrayProperty>(Prop);
	ArrProp->SetValue(Ctx, Params);
	return true;
}
