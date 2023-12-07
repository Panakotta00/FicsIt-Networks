#include "Reflection/FINUFunction.h"

#include "Misc/ScopeExit.h"

#include "tracy/Tracy.hpp"

TArray<FFINAnyNetworkValue> UFINUFunction::Execute(const FFINExecutionContext& Ctx,	const TArray<FFINAnyNetworkValue>& Params) const {
	ZoneScopedN("FINFunction Execute");
	
	if (RefFunction) {
		UObject* Obj = Ctx.GetObject();
		if (!Obj) throw FFINReflectionException(const_cast<UFINUFunction*>(this), "No valid object used for function execution.");
		
		TArray<FFINAnyNetworkValue> Output;
		// allocate & initialize parameter struct
		uint8* ParamStruct = (uint8*)FMemory_Alloca(RefFunction->GetStructureSize());
		if (ParamStruct) RefFunction->InitializeStruct(ParamStruct);
		ON_SCOPE_EXIT {
			if (ParamStruct) RefFunction->DestroyStruct(ParamStruct);
		};
		
		// copy parameters to parameter struct
		int i = 0;
		TArray<UFINProperty*> Properties = GetParameters();
		for (int j = 0; j < Properties.Num(); ++j) {
			UFINProperty* Param = Properties[j];
			if ((Param->GetPropertyFlags() & FIN_Prop_Param) && !(Param->GetPropertyFlags() & FIN_Prop_OutParam)) {
				if (Params.Num() <= i) throw FFINReflectionException(const_cast<UFINUFunction*>(this), FString::Printf(TEXT("Required parameter '%s' is not provided."), *Param->GetInternalName())); 
				Param->SetValue(ParamStruct, Params[i++]);
			}
		}
		if (GetFunctionFlags() & FIN_Func_VarArgs && Params.Num() > i) {
			TArray<FFINAnyNetworkValue> VarArgs = TArray(&Params[i], Params.Num()-i);
			VarArgsProperty->SetValue(ParamStruct, VarArgs);
			i = Params.Num();
		}

		{
			ZoneScopedN("UFunction Execute");
			Obj->ProcessEvent(RefFunction, ParamStruct);
		}

		// copy output parameters from paramter struct
		for (UFINProperty* Param : Parameters) {
			if ((Param->GetPropertyFlags() & FIN_Prop_Param) && (Param->GetPropertyFlags() & FIN_Prop_OutParam)) {
				Output.Add(Param->GetValue(ParamStruct));
			}
		}
		
		return Output;
	}
	
	return Super::Execute(Ctx, Params);
}
