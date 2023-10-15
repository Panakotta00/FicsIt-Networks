#pragma once

#include "Hologram/FGBuildableHologram.h"
#include "FINSizeablePanelHolo.generated.h"

UCLASS()
class AFINSizeablePanelHolo : public AFGBuildableHologram {
	GENERATED_BODY()

private:

public:
	UPROPERTY(Replicated)
	int PanelWidth = 1;

	UPROPERTY(Replicated)
	int PanelHeight = 1;
	
	int OldPanelHeight = 0;
	int OldPanelWidth = 0;
	FVector Normal;
	bool bSnapped = false;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;

	UPROPERTY(Replicated)
	bool bFinished = false;
	
	AFINSizeablePanelHolo();

	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor

	// Begin AFGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void SetHologramLocationAndRotation(const FHitResult& HitResult) override;
	virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) override;
	virtual void CheckValidFloor() override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	// End AFGBuildableHologram

	// Begin AFGHologram
	TArray< FItemAmount > GetBaseCost() const;
	// End AFGHologram

private:
	void ConstructParts();
};
