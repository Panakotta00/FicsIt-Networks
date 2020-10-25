#pragma once

#include "FINProperty.h"
#include "Network/FINParameterList.h"

#include "FINFunction.generated.h"

UENUM(BlueprintType)
enum EFINFunctionFlags {
	FIN_Func_None = 0b00000000,
	FIN_Func_VarArgs = 0b00000001,
};

inline EFINFunctionFlags operator|(EFINFunctionFlags Flags1, EFINFunctionFlags Flags2) {
	return (EFINFunctionFlags)(((uint8)Flags1) | ((uint8)Flags2));
}

UCLASS(BlueprintType)
class UFINFunction : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	FText Description;
	UPROPERTY()
	FString InternalName = TEXT("UnknownFunction");
	UPROPERTY()
	FText DisplayName = FText::FromString(TEXT("Unknown Function"));
	UPROPERTY()
	TArray<UFINProperty*> Parameters;
	UPROPERTY()
	UFunction* RefFunction = nullptr;
	TFunction<TArray<FFINAnyNetworkValue>(UObject*, const TArray<FFINAnyNetworkValue>&)> NativeFunction;
	UPROPERTY()
	TEnumAsByte<EFINFunctionFlags> FunctionFlags;
	
	/**
	 * Returns the description of this function
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FText GetDescription() const { return Description; }
	
	/**
	 * Returns a more cryptic name of the function, used mainly for internal reference
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FString GetInternalName() const { return InternalName; }

	/**
	 * Returns a human readable name of the function, mainly used for UI
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FText GetDisplayName() const { return DisplayName; }
	
	/**
	 * Returns a list of all the parameters this function has
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual TArray<UFINProperty*> GetParameters() const { return Parameters; }

	/**
	 * Returns the function flags of this function
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TEnumAsByte<EFINFunctionFlags> GetFunctionFlags() const { return FunctionFlags; }

	/**
	 * Executes the function with the given properties and the given object context
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<FFINAnyNetworkValue> Execute(UObject* Ctx, const TArray<FFINAnyNetworkValue>& Params) const {
		if (NativeFunction) return NativeFunction(Ctx, Params);
		if (RefFunction) {
			TArray<FFINAnyNetworkValue> Output;
			// allocate & initialize parameter struct
			uint8* ParamStruct = (uint8*)FMemory::Malloc(RefFunction->PropertiesSize);
			FMemory::Memzero(ParamStruct + RefFunction->ParmsSize, RefFunction->PropertiesSize - RefFunction->ParmsSize);
			RefFunction->InitializeStruct(ParamStruct);
			for (UProperty* LocalProp = RefFunction->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
				LocalProp->InitializeValue_InContainer(ParamStruct);
			}

			// copy parameters to parameter struct
			int i = 0;
			for (UFINProperty* Param : GetParameters()) {
				if ((Param->GetPropertyFlags() & FIN_Prop_Param) && !(Param->GetPropertyFlags() & FIN_Prop_OutParam)) {
					Param->SetValue(ParamStruct, Params[i++]);
				}
			}
			
			Ctx->ProcessEvent(RefFunction, ParamStruct);

			// copy output parameters from paramter struct
			for (UFINProperty* Param : GetParameters()) {
				if ((Param->GetPropertyFlags() & FIN_Prop_Param) && !(Param->GetPropertyFlags() & FIN_Prop_OutParam)) {
					Output.Add(Param->GetValue(ParamStruct));
				}
			}

			// destroy parameter struct
			for (UProperty* P = RefFunction->DestructorLink; P; P = P->DestructorLinkNext) {
				if (!P->IsInContainer(RefFunction->ParmsSize)) {
					P->DestroyValue_InContainer(ParamStruct);
				}
			}
			FMemory::Free(ParamStruct);
			return Output;
		}
		return TArray<FFINAnyNetworkValue>();
	}
};
