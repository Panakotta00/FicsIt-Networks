#pragma once
#include "FicsItNetworks/Network/FINNetworkCableHologram.h"
#include "FINMCPNetworkCableHologram.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINMCPNetworkCableHologram : public AFINNetworkCableHologram {
	GENERATED_BODY()
public:

	AFINMCPNetworkCableHologram();

	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	//virtual AActor* Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) override;
	virtual void SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hit) override;
	void UpdateMeshValidity(bool bValid);
	
	virtual int32 GetBaseCostMultiplier() const override;

	/** Location of the connector in world space */
	/** TODO: Remove const */
	virtual FVector GetConnectorPos(const FFINCablePlacementStepInfo* info) const;

	/** Rotation of the connector in world space */
	/** TODO: Remove const */
	virtual FRotator GetConnectorRot(const FFINCablePlacementStepInfo* info) const;
	
};
