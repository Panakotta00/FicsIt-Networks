#include "FIRExtendedValueType.h"

#include "FIRArrayProperty.h"
#include "FIRClassProperty.h"

#include "FIRStructProperty.h"
#include "FIRTraceProperty.h"
#include "FIRObjectProperty.h"

FFIRExtendedValueType::FFIRExtendedValueType(UFIRProperty* Property) {
	Type = Property->GetType();
	switch (Type) {
		case FIR_ARRAY:
			SubType = FFIRExtendedValueType(Cast<UFIRArrayProperty>(Property)->GetInnerType());
		break;
		case FIR_OBJ:
			RefSubType = FFicsItReflectionModule::Get().FindClass(Cast<UFIRObjectProperty>(Property)->GetSubclass());
		break;
		case FIR_CLASS:
			RefSubType = FFicsItReflectionModule::Get().FindClass(Cast<UFIRClassProperty>(Property)->GetSubclass());
		break;
		case FIR_TRACE:
			RefSubType = FFicsItReflectionModule::Get().FindClass(Cast<UFIRTraceProperty>(Property)->GetSubclass());
		break;
		case FIR_STRUCT:
			RefSubType = FFicsItReflectionModule::Get().FindStruct(Cast<UFIRStructProperty>(Property)->GetInner());
		break;
		default:
			RefSubType = nullptr;
	}
}

bool FFIRExtendedValueType::IsA(const FFIRExtendedValueType& Other) const {
	if (Other.Type == FIR_ANY) return true;
	if (Type != Other.Type) return false;
	if (Type == FIR_ARRAY) return SubType.Get<FFIRExtendedValueType>().IsA(Other.SubType.Get<FFIRExtendedValueType>());
	if (Type >= FIR_OBJ && Type != FIR_ANY) return RefSubType->IsChildOf(Other.RefSubType);
	return true;
}
