#pragma once
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGFactoryBuildingHologram.h"

#include "FINModularIndicatorPoleModuleHolo.generated.h"

class AFINIndicatorPole;
UCLASS()
class AFINModularIndicatorPoleModuleHolo : public AFGBuildableHologram {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FVector SnappedLoc;

	UPROPERTY()
	bool bSnapped = false;

	UPROPERTY()
	TWeakObjectPtr<AFGBuildable> SnappedObject;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Mesh;

	UPROPERTY(Replicated)
	bool bFinished = false;
	
	AFINModularIndicatorPoleModuleHolo();

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	//virtual bool CanBeZooped() const override;
//
	//virtual void ConstructZoop(TArray<AActor*>& out_children) override;
//
	//virtual void GetSupportedBuildModes_Implementation(TArray<TSubclassOf<UFGHologramBuildModeDescriptor>>& out_buildmodes) const override;

	FVector GetAttachPoint(AFGBuildable* Object) const;
};