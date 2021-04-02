#pragma once

#include "Hologram/FGBuildableHologram.h"
#include "FINScreenHolo.generated.h"

UCLASS()
class AFINScreenHolo : public AFGBuildableHologram {
	GENERATED_BODY()
	
public:
	UPROPERTY(Replicated)
	int ScreenWidth = 1;

	UPROPERTY(Replicated)
	int ScreenHeight = 1;
	
	int OldScreenHeight = 0;
	int OldScreenWidth = 0;
	FVector Normal;
	bool bSnapped = false;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;

	UPROPERTY(Replicated)
	bool bFinished = false;
	
	AFINScreenHolo();

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor

	// Begin AFGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) override;
	virtual void CheckValidFloor() override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	// End AFGBuildableHologram
};
