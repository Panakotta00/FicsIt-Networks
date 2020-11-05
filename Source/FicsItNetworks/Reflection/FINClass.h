#pragma once

#include "FINFunction.h"
#include "FINProperty.h"
#include "FINRefSignal.h"
#include "FINClass.generated.h"

UCLASS(BlueprintType)
class UFINClass : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	FText Description;
	UPROPERTY()
	FString InternalName = TEXT("UnknownClass");
	UPROPERTY()
	FText DisplayName = FText::FromString(TEXT("Unknown Class"));
	UPROPERTY()
	TArray<UFINProperty*> Properties;
	UPROPERTY()
	TArray<UFINFunction*> Functions;
	UPROPERTY()
	TArray<UFINRefSignal*> Signals;
	UPROPERTY()
	UFINClass* Parent = nullptr;
	
	/**
	 * Returns the description of this property
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FText GetDescription() const { return Description; }
	
	/**
	 * Returns a more cryptic name of the class, used mainly for internal reference
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual FString GetInternalName() const { return InternalName; }

	/**
	 * Returns a human readable name of the class, mainly used for UI
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual FText GetDisplayName() const { return DisplayName; }

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
	 * Returns a list of all available signals
	 * @param[in]	bRecursive	true if signals of all parent classes should be included
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINRefSignal*> GetSignals(bool bRecursive = true) const {
		TArray<UFINRefSignal*> Sigs = Signals;
		if (bRecursive && GetParent()) Sigs.Append(GetParent()->GetSignals());
		return Sigs;
	}

	/**
	 * Returns the parent class of this class
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual UFINClass* GetParent() const { return Parent; }

	/**
	 * Checks if this class extends from the given class (has the given class as an parent)
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	bool IsChildOf(const UFINClass* Class) const {
		const UFINClass* Self = this;
		while (Self) {
			if (Self == Class) return true;
			Self = Self->GetParent();
		}
		return false;
	}

	/**
	 * Returns a array filled with all direct child classes of this class.
	 */
	TArray<UFINClass*> GetChildClasses() const {
		TArray<UFINClass*> Childs;
		for(TObjectIterator<UFINClass> It; It; ++It) {
			if(It->GetParent() == this) {
				Childs.Add(*It);
			}
		}
		return Childs;
	}
};
