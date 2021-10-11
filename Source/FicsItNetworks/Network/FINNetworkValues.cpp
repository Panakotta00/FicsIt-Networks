#include "FINNetworkValues.h"

#include "FicsItNetworks/Reflection/FINArrayProperty.h"
#include "FicsItNetworks/Reflection/FINClassProperty.h"
#include "FicsItNetworks/Reflection/FINObjectProperty.h"
#include "FicsItNetworks/Reflection/FINReflection.h"
#include "FicsItNetworks/Reflection/FINTraceProperty.h"

FFINExpandedNetworkValueType::FFINExpandedNetworkValueType(UFINProperty* Property) {
	Type = Property->GetType();
	switch (Type) {
	case FIN_ARRAY:
		SubType = new FFINExpandedNetworkValueType(Cast<UFINArrayProperty>(Property)->GetInnerType());
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

bool FFINExpandedNetworkValueType::IsA(const FFINExpandedNetworkValueType& Other) {
	if (Type != Other.Type) return false;
	if (Type == FIN_ARRAY) return SubType->IsA(*Other.SubType);
	if (Type >= FIN_OBJ && Type != FIN_ANY) return RefSubType->IsChildOf(Other.RefSubType);
	return true;
}
