#include "Reflection/FIRUFunction.h"

#include "Misc/ScopeExit.h"
#include "Reflection/FIRArrayProperty.h"

#include "tracy/Tracy.hpp"

TArray<FFIRAnyValue> UFIRUFunction::Execute(const FFIRExecutionContext& Ctx,	const TArray<FFIRAnyValue>& Params) const {
	ZoneScopedN("FIRFunction Execute");
	
	if (RefFunction) {
		UObject* Obj = Ctx.GetObject();
		if (!Obj) throw FFIRReflectionException(const_cast<UFIRUFunction*>(this), "No valid object used for function execution.");
		
		TArray<FFIRAnyValue> Output;
		// allocate & initialize parameter struct
		uint8* ParamStruct = (uint8*)FMemory_Alloca(RefFunction->GetStructureSize());
		if (ParamStruct) RefFunction->InitializeStruct(ParamStruct);
		ON_SCOPE_EXIT {
			if (ParamStruct) RefFunction->DestroyStruct(ParamStruct);
		};
		
		// copy parameters to parameter struct
		int i = 0;
		TArray<UFIRProperty*> Properties = GetParameters();
		for (int j = 0; j < Properties.Num(); ++j) {
			UFIRProperty* Param = Properties[j];
			if ((Param->GetPropertyFlags() & FIR_Prop_Param) && !(Param->GetPropertyFlags() & FIR_Prop_OutParam)) {
				if (Params.Num() <= i) throw FFIRReflectionException(const_cast<UFIRUFunction*>(this), FString::Printf(TEXT("Required parameter '%s' is not provided."), *Param->GetInternalName()));
				Param->SetValue(ParamStruct, Params[i++]);
			}
		}
		if (GetFunctionFlags() & FIR_Func_VarArgs && Params.Num() > i) {
			TArray<FFIRAnyValue> VarArgs = TArray(&Params[i], Params.Num()-i);
			VarArgsProperty->SetValue(ParamStruct, VarArgs);
			i = Params.Num();
		}

		{
			ZoneScopedN("UFunction Execute");
			Obj->ProcessEvent(RefFunction, ParamStruct);
		}

		// copy output parameters from paramter struct
		for (UFIRProperty* Param : Parameters) {
			if ((Param->GetPropertyFlags() & FIR_Prop_Param) && (Param->GetPropertyFlags() & FIR_Prop_OutParam)) {
				Output.Add(Param->GetValue(ParamStruct));
			}
		}
		
		return Output;
	}
	
	return Super::Execute(Ctx, Params);
}
