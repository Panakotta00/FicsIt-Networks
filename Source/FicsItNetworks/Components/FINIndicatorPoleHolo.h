#pragma once

#include "FGBuildableHologram.h"
#include "FINIndicatorPoleHolo.generated.h"

class AFINIndicatorPole;
UCLASS()
class AFINIndicatorPoleHolo : public AFGBuildableHologram {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FVector SnappedLoc;

	UPROPERTY()
	bool bSnapped = false;

	UPROPERTY()
	TWeakObjectPtr<AFINIndicatorPole> SnappedPole;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Poles;

	UPROPERTY()
	int LastHeight = 0;
	
	AFINIndicatorPoleHolo();

	// Begin AFGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) override;
	virtual void CheckValidFloor() override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	// End AFGBuildableHologram

	int GetHeight(FVector worldLoc) const;
};