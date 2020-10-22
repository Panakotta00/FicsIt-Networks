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
    virtual FText GetDescription() { return Description; }
	
	/**
	 * Returns a more cryptic name of the class, used mainly for internal reference
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual FString GetInternalName() { return InternalName; }

	/**
	 * Returns a human readable name of the class, mainly used for UI
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual FText GetDisplayName() { return DisplayName; }

	/**
	 * Returns a list of all available properties
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINProperty*> GetProperties() { return Properties; }
	
	/**
	 * Returns a list of all available functions
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINFunction*> GetFunctions() { return Functions; }

	/**
	 * Returns a list of all available signals
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINRefSignal*> GetSignals() { return Signals; }

	/**
	 * Returns the parent class of this class
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual UFINClass* GetParent() { return Parent; }
};
