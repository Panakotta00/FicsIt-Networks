#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectIterator.h"
#include "FIRFunction.h"
#include "FIRStruct.generated.h"

UENUM()
enum EFIRStructFlags {
	FIR_Struct_None				= 0,
	FIR_Struct_StaticSource		= 0b01,
	FIR_Struct_Constructable	= 0b10,
};

ENUM_CLASS_FLAGS(EFIRStructFlags)

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRStruct : public UFIRBase {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<UFIRProperty*> Properties;
	UPROPERTY()
	TArray<UFIRFunction*> Functions;
	UPROPERTY()
	UFIRStruct* Parent = nullptr;
	
	EFIRStructFlags StructFlags = FIR_Struct_None;

	/**
	 * Returns the struct flags of this struct
	 */
	virtual EFIRStructFlags GetStructFlags() const { return StructFlags; }
	
	/**
	 * Returns a list of all available properties
	 * @param[in]	bRecursive	true if properties of all parent classes should be included
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFIRProperty*> GetProperties(bool bRecursive = true) const {
		TArray<UFIRProperty*> Props = Properties;
		if (bRecursive && GetParent()) Props.Append(GetParent()->GetProperties());
		return Props;
	}
	
	/**
	 * Returns a list of all available functions
	 * @param[in]	bRecursive	true if functions of all parent classes should be included
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFIRFunction*> GetFunctions(bool bRecursive = true) const {
		TArray<UFIRFunction*> Funcs = Functions;
		if (bRecursive && GetParent()) Funcs.Append(GetParent()->GetFunctions());
		return Funcs;
	}
	
	/**
	 * Returns the parent class of this class
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual UFIRStruct* GetParent() const { return Parent; }

	/**
	 * Checks if this class extends from the given class (has the given struct as an parent)
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	bool IsChildOf(const UFIRStruct* Struct) const {
		if (!Struct) return true;
		const UFIRStruct* Self = this;
		while (Self) {
			if (Self == Struct) return true;
			Self = Self->GetParent();
		}
		return false;
	}

	/**
	 * Returns a array filled with all direct child classes of this strzct.
	 */
	TArray<UFIRStruct*> GetChildren() const {
		TArray<UFIRStruct*> Childs;
		for(TObjectIterator<UFIRStruct> It; It; ++It) {
			if(It->GetParent() == this) {
				Childs.Add(*It);
			}
		}
		return Childs;
	}

	/**
	 * Trys to find a property with the given name
	 */
	UFIRProperty* FindFIRProperty(const FString& Name, EFIRPropertyFlags FilterFlags = FIR_Prop_Attrib);

	/**
	 * Trys to find a function with the given name
	 */
	UFIRFunction* FindFIRFunction(const FString& Name, EFIRFunctionFlags FilterFlags = FIR_Func_MemberFunc);

	UFUNCTION(BlueprintCallable)
	virtual void InvalidateCache() {
		FScopeLock nameCacheLock(&NameCacheMutex);
		Name2Property.Empty();
		Name2Function.Empty();
	}

private:
	FCriticalSection NameCacheMutex;
	TMap<FString, TArray<UFIRProperty*>> Name2Property;
	TMap<FString, TArray<UFIRFunction*>> Name2Function;
};
