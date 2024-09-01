#include "FIRUtils.h"
#include "Reflection/FIRArrayProperty.h"
#include "Reflection/FIRBoolProperty.h"
#include "Reflection/FIRClassProperty.h"
#include "Reflection/FIRFloatProperty.h"
#include "Reflection/FIRIntProperty.h"
#include "Reflection/FIRObjectProperty.h"
#include "Reflection/FIRStrProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/FIRTraceProperty.h"

bool UFIRUtils::CheckIfVarargs(UFIRProperty* Prop) {
	if (!Prop->GetInternalName().Equals("varargs")) return false;
	UFIRArrayProperty* ArrayProperty = Cast<UFIRArrayProperty>(Prop);
	if (!ArrayProperty || !ArrayProperty->InnerType) return false;
	UFIRStructProperty* StructProperty = Cast<UFIRStructProperty>(ArrayProperty->InnerType);
	if (!StructProperty || StructProperty->Struct != FFIRAnyValue::StaticStruct()) return false;
	return true;
}

bool UFIRUtils::SetIfVarargs(UFIRProperty* Prop, const FFIRExecutionContext& Ctx, const TArray<FFIRAnyValue>& Params) {
	if (!CheckIfVarargs(Prop)) return false;
	UFIRArrayProperty* ArrProp = Cast<UFIRArrayProperty>(Prop);
	ArrProp->SetValue(Ctx, Params);
	return true;
}


UFIRProperty* FIRCreateFIRPropertyFromFProperty(FProperty* Property, FProperty* OverrideProperty, UObject* Outer) {
	UFIRProperty* FIRProp = nullptr;
	if (CastField<FStrProperty>(Property)) {
		UFIRStrProperty* FIRStrProp = NewObject<UFIRStrProperty>(Outer);
		FIRStrProp->Property = CastField<FStrProperty>(OverrideProperty);
		FIRProp = FIRStrProp;
	} else if (CastField<FIntProperty>(Property)) {
		UFIRIntProperty* FIRIntProp = NewObject<UFIRIntProperty>(Outer);
		FIRIntProp->Property = CastField<FIntProperty>(OverrideProperty);
		FIRProp = FIRIntProp;
	} else if (CastField<FByteProperty>(Property)) {
		UFIRIntProperty* FIRIntProp = NewObject<UFIRIntProperty>(Outer);
		FIRIntProp->Property8 = CastField<FByteProperty>(OverrideProperty);
		FIRProp = FIRIntProp;
	} else if (CastField<FInt64Property>(Property)) {
		UFIRIntProperty* FIRIntProp = NewObject<UFIRIntProperty>(Outer);
		FIRIntProp->Property64 = CastField<FInt64Property>(OverrideProperty);
		FIRProp = FIRIntProp;
	} else if (CastField<FFloatProperty>(Property)) {
		UFIRFloatProperty* FIRFloatProp = NewObject<UFIRFloatProperty>(Outer);
		FIRFloatProp->FloatProperty = CastField<FFloatProperty>(OverrideProperty);
		FIRProp = FIRFloatProp;
	} else if (CastField<FDoubleProperty>(Property)) {
		UFIRFloatProperty* FIRFloatProp = NewObject<UFIRFloatProperty>(Outer);
		FIRFloatProp->DoubleProperty = CastField<FDoubleProperty>(OverrideProperty);
		FIRProp = FIRFloatProp;
	} else if (CastField<FBoolProperty>(Property)) {
		UFIRBoolProperty* FIRBoolProp = NewObject<UFIRBoolProperty>(Outer);
		FIRBoolProp->Property = CastField<FBoolProperty>(OverrideProperty);
		FIRProp = FIRBoolProp;
	} else if (CastField<FClassProperty>(Property)) {
		UFIRClassProperty* FIRClassProp = NewObject<UFIRClassProperty>(Outer);
		FIRClassProp->Property = CastField<FClassProperty>(OverrideProperty);
		FIRProp = FIRClassProp;
	} else if (CastField<FObjectProperty>(Property)) {
		UFIRObjectProperty* FIRObjectProp = NewObject<UFIRObjectProperty>(Outer);
		FIRObjectProp->Property = CastField<FObjectProperty>(OverrideProperty);
		FIRProp = FIRObjectProp;
	} else  if (CastField<FStructProperty>(Property)) {
		FStructProperty* StructProp = CastField<FStructProperty>(OverrideProperty);
		if (StructProp->Struct == FFIRTrace::StaticStruct()) {
			UFIRTraceProperty* FIRTraceProp = NewObject<UFIRTraceProperty>(Outer);
			FIRTraceProp->Property = StructProp;
			FIRProp = FIRTraceProp;
		} else {
			UFIRStructProperty* FIRStructProp = NewObject<UFIRStructProperty>(Outer);
			checkf(StructProp->Struct == FFIRAnyNetworkValue::StaticStruct() || FFicsItReflectionModule::Get().FindStruct(StructProp->Struct) != nullptr, TEXT("Struct Property '%s' of reflection-base '%s' uses non-reflectable struct '%s'"), *Property->GetFullName(), *Outer->GetName(), *StructProp->Struct->GetName());
			FIRStructProp->Property = StructProp;
			FIRStructProp->Struct = StructProp->Struct;
			FIRProp = FIRStructProp;
		}
    } else if (CastField<FArrayProperty>(Property)) {
    	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(OverrideProperty);
	    UFIRArrayProperty* FIRArrayProp = NewObject<UFIRArrayProperty>(Outer);
    	FIRArrayProp->Property = ArrayProperty;
    	FIRArrayProp->InnerType = FIRCreateFIRPropertyFromFProperty(ArrayProperty->Inner, FIRArrayProp);
    	FIRProp = FIRArrayProp;
    }
	check(FIRProp != nullptr);
	if (Property->PropertyFlags & CPF_OutParm) FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_OutParam;
	if (Property->PropertyFlags & CPF_ReturnParm) FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_RetVal;
	return FIRProp;
}
