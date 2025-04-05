#pragma once

#include "CoreMinimal.h"
#include "FINModuleSystemPanel.h"
#include "Components/TextRenderComponent.h"
#include "Hologram/FGBuildableHologram.h"
#include "FINModuleSystemHolo.generated.h"

UCLASS()
class FICSITNETWORKSMISC_API AFINModuleSystemHolo : public AFGBuildableHologram {
	GENERATED_BODY()

public:
	UPROPERTY()
	class UFINModuleSystemPanel* Snapped = nullptr;
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
	virtual void Destroyed() override;
	virtual void OnInvalidHitResult() override;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTextRenderComponent* InformationComponent;

	UPROPERTY(EditAnywhere)
	bool EnableInformationDisplay = false;
	
	UPROPERTY(EditAnywhere)
	FVector InformationDisplayOffset = FVector(1,6.5,7);

	UFUNCTION(BlueprintImplementableEvent)
	void OnInformationUpdate(UTextRenderComponent* Component, const FHitResult& HitResult, UFINModuleSystemPanel* SnappedPanel, const FVector ModuleLocation, const int ModuleRotation);
	
private:
	bool checkSpace(FVector min, FVector max);
	
	FVector getModuleSize();

	UPROPERTY()
	UStaticMeshComponent* CompassRose = nullptr;
};