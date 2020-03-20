#pragma once

#include "CoreMinimal.h"
#include "FGBuildableHologram.h"
#include "FINModuleSystemPanel.h"
#include "FINModuleSystemHolo.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINModuleSystemHolo : public AFGBuildableHologram {
	GENERATED_BODY()

public:
	UFINModuleSystemPanel* Snapped = nullptr;
	FVector SnappedLoc;
	int SnappedRot;
	bool bIsValid;

	AFINModuleSystemHolo();
	~AFINModuleSystemHolo();

	// Begin AFGHologram
	virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID constructionID);
	virtual bool IsValidHitResult(const FHitResult& hit) const override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hit) override;
	virtual void CheckValidPlacement() override;
	// ENd AFGHologram

private:
	bool checkSpace(FVector min, FVector max);
	FVector getModuleSize();
};