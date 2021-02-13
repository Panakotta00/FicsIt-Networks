#pragma once

#include "FINBase.h"
#include "FINFunction.h"
#include "UObjectIterator.h"

#include "FINStruct.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINStruct : public UFINBase {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<UFINProperty*> Properties;
	UPROPERTY()
	TArray<UFINFunction*> Functions;
	UPROPERTY()
	UFINStruct* Parent = nullptr;
	
	/**
	 * Returns a list of all available properties
	 * @param[in]	bRecursive	true if properties of all parent classes should be included
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINProperty*> GetProperties(bool bRecursive = true) const {
		TArray<UFINProperty*> Props = Properties;
		if (bRecursive && GetParent()) Props.Append(GetParent()->GetProperties());
		return Props;
	}
	
	/**
	 * Returns a list of all available functions
	 * @param[in]	bRecursive	true if functions of all parent classes should be included
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINFunction*> GetFunctions(bool bRecursive = true) const {
		TArray<UFINFunction*> Funcs = Functions;
		if (bRecursive && GetParent()) Funcs.Append(GetParent()->GetFunctions());
		return Funcs;
	}
	
	/**
	 * Returns the parent class of this class
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual UFINStruct* GetParent() const { return Parent; }

	/**
	 * Checks if this class extends from the given class (has the given struct as an parent)
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	bool IsChildOf(const UFINStruct* Struct) const {
		const UFINStruct* Self = this;
		while (Self) {
			if (Self == Struct) return true;
			Self = Self->GetParent();
		}
		return false;
	}

	/**
	 * Returns a array filled with all direct child classes of this strzct.
	 */
	TArray<UFINStruct*> GetChildren() const {
		TArray<UFINStruct*> Childs;
		for(TObjectIterator<UFINStruct> It; It; ++It) {
			if(It->GetParent() == this) {
				Childs.Add(*It);
			}
		}
		return Childs;
	}

	/**
	 * Trys to find a property with the given name
	 */
	UFINProperty* FindFINProperty(const FString& Name, EFINRepPropertyFlags FilterFlags = FIN_Prop_Attrib);

	/**
	 * Trys to find a function with the given name
	 */
	UFINFunction* FindFINFunction(const FString& Name, EFINFunctionFlags FilterFlags = FIN_Func_MemberFunc);
};
