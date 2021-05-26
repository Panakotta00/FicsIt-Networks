#pragma once

#include "CoreMinimal.h"
#include "FINNetworkAdapter.h"
#include "FINNetworkConnectionComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Hologram/FGBuildableHologram.h"

#include "FINNetworkCableHologram.generated.h"

UENUM()
enum EFINNetworkCableHologramSnapType {
	FIN_NOT_SNAPPED,
	FIN_CONNECTOR,
	FIN_SETTINGS,
	FIN_POWER,
	FIN_POLE,
};

USTRUCT()
struct FICSITNETWORKS_API FFINSnappedInfo {
	GENERATED_BODY()

public:
	EFINNetworkCableHologramSnapType SnapType = FIN_NOT_SNAPPED;
	UObject* SnappedObj = nullptr;
	FVector PolePostition;
	FFINAdapterSettings AdapterSettings;

	/** Location of the connector in world space */
	FVector GetConnectorPos() const;

	/** Rotation of the connector in world space */
	FRotator GetConnectorRot() const;
	
	AActor* GetActor() const;
};

UCLASS()
class FICSITNETWORKS_API AFINNetworkCableHologram : public AFGBuildableHologram {
	GENERATED_BODY()

public:
	UPROPERTY()
	USplineMeshComponent* Cable = nullptr;

	UPROPERTY()
	UStaticMeshComponent* Adapter1 = nullptr;

	UPROPERTY()
	UStaticMeshComponent* Adapter2 = nullptr;

	UPROPERTY(Replicated)
	FFINSnappedInfo Snapped;

	UPROPERTY(Replicated)
	FFINSnappedInfo OldSnapped;

	UPROPERTY(Replicated)
	FFINSnappedInfo From;

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

	AFINNetworkCableHologram();
	~AFINNetworkCableHologram();

	void OnBeginSnap(FFINSnappedInfo a, bool isValid);
	void OnEndSnap(FFINSnappedInfo a);
	UFINNetworkConnectionComponent* SetupSnapped(FFINSnappedInfo s);
	void UpdateSnapped();
	
	/**
	 * Checks if the currently snapped object is valid
	 */
	bool IsSnappedValid();
};