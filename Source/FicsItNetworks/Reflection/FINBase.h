#pragma once

#include "CoreMinimal.h"
#include "FINBase.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINBase : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	FText Description;
	UPROPERTY()
	FString InternalName = TEXT("Unknown_") + StaticClass()->GetName();
	UPROPERTY()
	FText DisplayName = FText::FromString(TEXT("Unknown ") + StaticClass()->GetName());
	
	/**
	 * Returns the description
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FText GetDescription() const { return Description; }
	
	/**
	 * Returns a more cryptic name, used mainly for internal reference
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FString GetInternalName() const { return InternalName; }

	/**
	 * Returns a human readable name, mainly used for UI
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FText GetDisplayName() const { return DisplayName; }
};
