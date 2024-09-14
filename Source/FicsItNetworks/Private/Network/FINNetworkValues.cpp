#include "Network/FINNetworkValues.h"

#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"

FFINExpandedNetworkValueType::FFINExpandedNetworkValueType(UFINProperty* Property) {
	Type = Property->GetType();
	switch (Type) {
	case FIN_ARRAY:
		SubType = FFINExpandedNetworkValueType(Cast<UFINArrayProperty>(Property)->GetInnerType());
		break;
	case FIN_OBJ:
		RefSubType = FFINReflection::Get()->FindClass(Cast<UFINObjectProperty>(Property)->GetSubclass());
		break;
	case FIN_CLASS:
		RefSubType = FFINReflection::Get()->FindClass(Cast<UFINClassProperty>(Property)->GetSubclass());
		break;
	case FIN_TRACE:
		RefSubType = FFINReflection::Get()->FindClass(Cast<UFINTraceProperty>(Property)->GetSubclass());
		break;
	case FIN_STRUCT:
		RefSubType = FFINReflection::Get()->FindStruct(Cast<UFINStructProperty>(Property)->GetInner());
		break;
	default:
		RefSubType = nullptr;
	}
}

bool FFINExpandedNetworkValueType::IsA(const FFINExpandedNetworkValueType& Other) const {
	if (Other.Type == FIN_ANY) return true;
	if (Type != Other.Type) return false;
	if (Type == FIN_ARRAY) return SubType.Get<FFINExpandedNetworkValueType>().IsA(Other.SubType.Get<FFINExpandedNetworkValueType>());
	if (Type >= FIN_OBJ && Type != FIN_ANY) return RefSubType->IsChildOf(Other.RefSubType);
	return true;
}