#pragma once

#include "FINArrayProperty.h"
#include "FINFunction.h"
#include "FINUFunction.generated.h"

UCLASS()
class UFINUFunction : public UFINFunction {
	GENERATED_BODY()
public:
	UPROPERTY()
	UFunction* RefFunction = nullptr;
	UPROPERTY()
	UFINArrayProperty* VarArgsProperty = nullptr;

	// Begin UFINFunction
	virtual TArray<FFINAnyNetworkValue> Execute(const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params) const override {
		if (RefFunction) {
			UObject* Obj = Ctx.GetObject();
			if (!Obj) throw FFINReflectionException(const_cast<UFINUFunction*>(this), "No valid object used for function execution.");
			
			TArray<FFINAnyNetworkValue> Output;
			// allocate & initialize parameter struct
			uint8* ParamStruct = (uint8*)FMemory_Alloca(RefFunction->PropertiesSize);
			RefFunction->InitializeStruct(ParamStruct);
			
			// copy parameters to parameter struct
			int i = 0;
			TArray<UFINProperty*> Parameters = GetParameters();
			for (int j = 0; j < Parameters.Num(); ++j) {
				UFINProperty* Param = Parameters[j];
				if ((Param->GetPropertyFlags() & FIN_Prop_Param) && !(Param->GetPropertyFlags() & FIN_Prop_OutParam)) {
					if (Params.Num() <= i) throw FFINReflectionException(const_cast<UFINUFunction*>(this), FString::Printf(TEXT("Required parameter '%s' is not provided."), *Param->GetInternalName())); 
					Param->SetValue(ParamStruct, Params[i++]);
				}
			}
			if (GetFunctionFlags() & FIN_Func_VarArgs && Params.Num() > i+1) {
				TArray<FFINAnyNetworkValue> VarArgs = TArray<FFINAnyNetworkValue>(&Params[i], Params.Num()-i);
				VarArgsProperty->SetValue(ParamStruct, VarArgs);
				i = Params.Num();
			}
			
			Obj->ProcessEvent(RefFunction, ParamStruct);

			// copy output parameters from paramter struct
			for (UFINProperty* Param : GetParameters()) {
				if ((Param->GetPropertyFlags() & FIN_Prop_Param) && (Param->GetPropertyFlags() & FIN_Prop_OutParam)) {
					Output.Add(Param->GetValue(ParamStruct));
				}
			}

			// destroy parameter struct
			for (UProperty* P = RefFunction->DestructorLink; P; P = P->DestructorLinkNext) {
				if (!P->IsInContainer(RefFunction->ParmsSize)) {
					P->DestroyValue_InContainer(ParamStruct);
				}
			}
			return Output;
		}

		return Super::Execute(Ctx, Params);
	}
};
