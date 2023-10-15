#include "Reflection/FINProperty.h"

FINAny FFINPropertyGetterFunc::operator()(const FFINExecutionContext& Ctx, bool* Done) const {
	if (Function) {
		if (Done) *Done = true;
		UObject* Obj = Ctx.GetObject();
		check(Property != nullptr);
		check(Obj != nullptr);
		uint8* Params = (uint8*)FMemory::Malloc(Function->PropertiesSize);
		FMemory::Memzero(Params + Function->ParmsSize, Function->PropertiesSize - Function->ParmsSize);
		/*Function->InitializeStruct(Params);
		for (UProperty* LocalProp = Function->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Params);
		}*/
		for (TFieldIterator<FProperty> Prop(Function); Prop; ++Prop) {
			if (Prop->GetPropertyFlags() & CPF_Parm) Prop->InitializeValue_InContainer(Params);
		}
		Obj->ProcessEvent(Function, Params);
		FINAny Return = Property->GetValue(FFINExecutionContext(Params));
		for (FProperty* P = Function->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(Function->ParmsSize)) {
				P->DestroyValue_InContainer(Params);
			}
		}
		FMemory::Free(Params);
		return Return;
	}
	if (GetterFunc) {
		if (Done) *Done = true;
		return GetterFunc(Ctx);
	}
	if (Done) *Done = false;
	return FINAny();
}

bool FFINPropertySetterFunc::operator()(const FFINExecutionContext& Ctx, const FINAny& Any) const {
	if (Function) {
		UObject* Obj = Ctx.GetObject();
		check(Obj != nullptr);
		check(Property != nullptr);
		uint8* Params = (uint8*)FMemory::Malloc(Function->PropertiesSize);
		FMemory::Memzero(Params + Function->ParmsSize, Function->PropertiesSize - Function->ParmsSize);
		for (TFieldIterator<FProperty> Prop(Function); Prop; ++Prop) {
			if (Prop->GetPropertyFlags() & CPF_Parm) Prop->InitializeValue_InContainer(Params);
		}
		Property->SetValue(FFINExecutionContext(Params), Any);
		Obj->ProcessEvent(Function, Params);
		for (FProperty* P = Function->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(Function->ParmsSize)) {
				P->DestroyValue_InContainer(Params);
			}
		}
		FMemory::Free(Params);
		return true;
	}
	if (SetterFunc) {
		SetterFunc(Ctx, Any);
		return true;
	}
	return false;
}
