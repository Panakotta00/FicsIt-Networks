#pragma once

#include "FGBuildableFactoryBuilding.h"
#include "Hologram/FGBuildableHologram.h"
#include "FINWallAndFoundationHologram.generated.h"

UCLASS()
class AFINWallAndFoundationHologram : public AFGBuildableHologram {
	GENERATED_BODY()
public:
	static EFoundationSide GetHitSide(FTransform hitTransform, FVector_NetQuantizeNormal hitNormal);
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void CheckValidFloor() override;
};
