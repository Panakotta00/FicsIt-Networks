#pragma once

#include "Hologram/FGBuildableHologram.h"
#include "FINNetworkAdapterHologram.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINNetworkAdapterHologram : public AFGBuildableHologram {
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bSnapped = false;
	
	UPROPERTY()
	FTransform SnappedTransform;

	UPROPERTY()
	bool bNeedsMesh = false;

	UPROPERTY()
	int MaxCables = 0;

	UPROPERTY()
	AActor* PrevSnappedActor = nullptr;

	UPROPERTY()
	TArray<USceneComponent*> CachedComponents;
	
	AFINNetworkAdapterHologram();

	// Begin AFGBuildableHologram
	virtual USceneComponent* SetupComponent(USceneComponent* attachParent, UActorComponent* componentTemplate, const FName& componentName, const FName& socketName) override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void OnInvalidHitResult() override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	// End AFGBuildableHologram

	void UpdateSnapped(AActor* NewSnappedActor);
	void OnBeginSnap(AActor* ActorSnapped);
	void OnEndSnap(AActor* ActorUnsapped);
};
