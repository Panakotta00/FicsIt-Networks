#pragma once
#include "FINModularIndicatorPole.h"
#include "Hologram/FGBuildableHologram.h"

#include "FINModularIndicatorPoleHolo.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworks_DebugRoze, Log, Log);

class AFINModularIndicatorPoleHolo;
UCLASS()
class AFINModularIndicatorPoleHolo : public AFGBuildableHologram {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FVector SnappedLoc;

	UPROPERTY()
	bool bSnapped = false;

	UPROPERTY(Replicated)
	bool bFinished = false;

	
	UPROPERTY(Replicated)
	int Extension = 1;
	
	UPROPERTY(Replicated)
	bool Vertical = false;
	
	
	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;
	FVector Normal;
	bool LastVertical;
	int LastExtension = 0;
	
	FRotator FloorOrientationModifier = FRotator(90,0,0);

	AFINModularIndicatorPoleHolo();

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	// End AActor

	// Begin AFGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	virtual void CheckValidFloor() override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	// End AFGBuildableHologram

	int GetHeight(FVector worldLoc) const;
};