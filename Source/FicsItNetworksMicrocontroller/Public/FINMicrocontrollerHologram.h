#pragma once

#include "CoreMinimal.h"
#include "FGBuildableHologram.h"
#include "FGConstructDisqualifier.h"
#include "FINMicrocontrollerHologram.generated.h"

UCLASS()
class FICSITNETWORKSMICROCONTROLLER_API UFINCDExistingMicrocontroller : public UFGConstructDisqualifier {
	GENERATED_BODY()

	UFINCDExistingMicrocontroller() {
		mDisqfualifyingText = FText::FromString(TEXT("Microcontroller already attached"));
	}
};

UCLASS()
class FICSITNETWORKSMICROCONTROLLER_API AFINMicrocontrollerHologram : public AFGBuildableHologram {
	GENERATED_BODY()
public:
	// Begin AActor
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor

	// Begin AFGHologram
	virtual bool IsValidHitActor(AActor* hitActor) const override;
	virtual void OnInvalidHitResult() override;
	// End AFGHologram

	// Begin AFGBuildableHologram
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual void CheckValidPlacement() override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	// End AFGBuildableHologram

protected:
	UPROPERTY()
	AActor* SnappedNetworkComponent = nullptr;
};
