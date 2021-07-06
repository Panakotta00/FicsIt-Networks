#pragma once

#include "Hologram/FGBuildableHologram.h"
#include "FINWallAndFoundationHologram.generated.h"

UCLASS()
class AFINWallAndFoundationHologram : public AFGBuildableHologram {
	GENERATED_BODY()
public:
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void CheckValidFloor() override;
};
