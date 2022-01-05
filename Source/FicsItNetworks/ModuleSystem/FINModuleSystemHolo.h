#pragma once

#include "CoreMinimal.h"
#include "FINModuleSystemPanel.h"
#include "Hologram/FGBuildableHologram.h"
#include "FINModuleSystemHolo.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINModuleSystemHolo : public AFGBuildableHologram {
	GENERATED_BODY()

public:
	UPROPERTY()
	UFINModuleSystemPanel* Snapped = nullptr;
	FVector SnappedLoc;
	int SnappedRot;

	//UPROPERTY(Replicated)
	bool bIsValid = false;
	bool bOldIsValid = false;

	AFINModuleSystemHolo();
	~AFINModuleSystemHolo();

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	// Begin AFGHologram
	virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID constructionID) override;
	virtual bool IsValidHitResult(const FHitResult& hit) const override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hit) override;
	virtual void CheckValidPlacement() override;
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	// End AFGHologram

	UPROPERTY(EditAnywhere)
	FVector CompassSurfaceOffset = FVector(1,5,7);
	
	UPROPERTY(EditAnywhere)
	FRotator CompassRotation = FRotator(0.000000,90.000000,-90.000000);

	UPROPERTY(EditAnywhere)
	FVector CompassScale = FVector(0.5f, 0.5f, 0.5f);

	UPROPERTY(EditAnywhere)
	UStaticMesh* CompassMesh = nullptr;

	UPROPERTY(EditAnywhere)
	bool ShowCompass = false;
	
private:
	bool checkSpace(FVector min, FVector max);
	FVector getModuleSize();

	UPROPERTY()
	UStaticMeshComponent* CompassRose = nullptr;
};