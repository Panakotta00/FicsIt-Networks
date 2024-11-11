#pragma once

#include "CoreMinimal.h"
#include "FGAvailabilityDependency.h"
#include "FGPlayerState.h"
#include "FIRSubsystem.h"
#include "FINDependencies.generated.h"

UCLASS(Blueprintable)
class UFINDependency : public UFGAvailabilityDependency {
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Dependency")
	FText Description;

	// Begin UFGAvailabilityDependency
	virtual bool AreDependenciesMet(UObject* worldContext) const override;
	// End UFGAvailabilityDependency

	UFUNCTION(BlueprintCallable)
	virtual FText GetDescription() const { return Description; }
};