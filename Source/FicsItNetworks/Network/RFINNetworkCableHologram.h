#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGBuildableHologram.h"
#include "Buildables/FGBuildableFactory.h"
#include "FicsItNetworks/Network/FINNetworkAdapter.h"
#include "FicsItNetworks/Network/FINNetworkConnectionComponent.h"
#include "FicsItNetworks/Network/FINNetworkCableHologram.h"
#include "Components/SplineMeshComponent.h"
#include "RFINNetworkCableHologram.generated.h"

UCLASS()
class ARFINNetworkCableHologram : public AFGBuildableHologram {
	GENERATED_BODY()

public:
	UPROPERTY()
	USplineMeshComponent* Cable = nullptr;

	UPROPERTY()
	UStaticMeshComponent* Adapter1 = nullptr;

	UPROPERTY()
	UStaticMeshComponent* Adapter2 = nullptr;

	UPROPERTY(Replicated)
	FFINCablePlacementStepInfo Snapped;

	UPROPERTY(Replicated)
	FFINCablePlacementStepInfo OldSnapped;

	UPROPERTY(Replicated)
	FFINCablePlacementStepInfo From;

	UPROPERTY()
	AFGBuildableHologram* PoleHologram = nullptr;

	UPROPERTY()
	UFINNetworkConnectionComponent* Connector1Cache = nullptr;

	UPROPERTY()
	UFINNetworkConnectionComponent* Connector2Cache = nullptr;

	// Begin FGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual AActor* Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool IsValidHitResult(const FHitResult& hit) const override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hit) override;
	virtual void OnInvalidHitResult() override;
	virtual bool IsChanged() const override;
	virtual USceneComponent* SetupComponent(USceneComponent* attachParent, UActorComponent* templateComponent, const FName& componentName) override;
	virtual void SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) override;
	// End FGBuildableHologram

	ARFINNetworkCableHologram();
	~ARFINNetworkCableHologram();

	void OnBeginSnap(FFINCablePlacementStepInfo a, bool isValid);
	void OnEndSnap(FFINCablePlacementStepInfo a);
	UFINNetworkConnectionComponent* SetupSnapped(FFINCablePlacementStepInfo s);
	void UpdateSnapped();
	
	/**
	 * Checks if the currently snapped object is valid
	 */
	bool IsSnappedValid();
};