#pragma once

#include "FINArrayProperty.h"
#include "FINFunction.h"
#include "FINUFunction.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINUFunction : public UFINFunction {
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
			for (TFieldIterator<UProperty> Prop(RefFunction); Prop; ++Prop) {
				if (Prop->GetPropertyFlags() & CPF_Parm) {
					if (Prop->IsInContainer(RefFunction->ParmsSize)) Prop->InitializeValue_InContainer(ParamStruct);
				}
			}
			
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
				if (P->IsInContainer(RefFunction->ParmsSize)) {
					P->DestroyValue_InContainer(ParamStruct);
				}
			}
			return Output;
		}

		return Super::Execute(Ctx, Params);
	}
};
