#include "Reflection/FIRProperty.h"

#include "Misc/ScopeExit.h"

FIRAny FFIRPropertyGetterFunc::operator()(const FFIRExecutionContext& Ctx, bool* Done) const {
	if (Function) {
		if (Done) *Done = true;
		UObject* Obj = Ctx.GetObject();
		check(Property != nullptr);
		check(Obj != nullptr);
		uint8* Params = (uint8*)FMemory_Alloca(Function->GetStructureSize());
		if (Params) Function->InitializeStruct(Params);
		ON_SCOPE_EXIT {
			if (Params) Function->DestroyStruct(Params);
		};
		
		Obj->ProcessEvent(Function, Params);

		FIRAny Return = Property->GetValue(FFIRExecutionContext(Params));
		return Return;
	}
	if (GetterFunc) {
		if (Done) *Done = true;
		return GetterFunc(Ctx);
	}
	if (Done) *Done = false;
	return FIRAny();
}

bool FFIRPropertySetterFunc::operator()(const FFIRExecutionContext& Ctx, const FIRAny& Any) const {
	if (Function) {
		UObject* Obj = Ctx.GetObject();
		check(Obj != nullptr);
		check(Property != nullptr);
		uint8* Params = (uint8*)FMemory_Alloca(Function->GetStructureSize());
		if (Params) Function->InitializeStruct(Params);
		ON_SCOPE_EXIT{
			if (Params) Function->DestroyStruct(Params);
		};
		
		Property->SetValue(FFIRExecutionContext(Params), Any);

		Obj->ProcessEvent(Function, Params);
		
		return true;
	}
	if (SetterFunc) {
		SetterFunc(Ctx, Any);
		return true;
	}
	return false;
}
