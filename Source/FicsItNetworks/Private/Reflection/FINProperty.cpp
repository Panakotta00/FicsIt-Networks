#include "Reflection/FINProperty.h"

#include "Misc/ScopeExit.h"

FINAny FFINPropertyGetterFunc::operator()(const FFINExecutionContext& Ctx, bool* Done) const {
	if (Function) {
		if (Done) *Done = true;
		UObject* Obj = Ctx.GetObject();
		check(Property != nullptr);
		check(Obj != nullptr);
		uint8* Params = (uint8*)FMemory_Alloca(Function->GetStructureSize());
		Function->InitializeStruct(Params);
		ON_SCOPE_EXIT {
			Function->DestroyStruct(Params);
		};
		
		Obj->ProcessEvent(Function, Params);

		FINAny Return = Property->GetValue(FFINExecutionContext(Params));
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
		uint8* Params = (uint8*)FMemory_Alloca(Function->GetStructureSize());
		Function->InitializeStruct(Params);
		ON_SCOPE_EXIT{
			Function->DestroyStruct(Params);
		};
		
		Property->SetValue(FFINExecutionContext(Params), Any);

		Obj->ProcessEvent(Function, Params);
		
		return true;
	}
	if (SetterFunc) {
		SetterFunc(Ctx, Any);
		return true;
	}
	return false;
}
