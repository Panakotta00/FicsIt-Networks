// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGConstructDisqualifier.h"
#include "GameFramework/Actor.h"
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGHologram.h"
#include "FINWirelessAccessPointHologram.generated.h"

// TODO Currently not implemented
enum class EFINWirelessAccessPointBuildStep : uint8 {
	Place = 0 UMETA(DisplayName = "Place"),
	Rotate = 1 UMETA(DisplayName = "Rotate")
};

#define LOCTEXT_NAMESPACE "Construct Disqualifier FIN"

UCLASS()
class FICSITNETWORKS_API UFGCDWirelessAccessPointRequiresTower : public UFGConstructDisqualifier {
	GENERATED_BODY()

	UFGCDWirelessAccessPointRequiresTower() {
		mDisqfualifyingText = LOCTEXT( "UFGCDWirelessAccessPointRequiresTower", "Must snap to a Radio Tower!" );
	}
};

#undef LOCTEXT_NAMESPACE

UCLASS()
class FICSITNETWORKS_API AFINWirelessAccessPointHologram : public AFGBuildableHologram {
	GENERATED_BODY()

public:
	UPROPERTY()
	bool IsSnapped = false;
	
	UPROPERTY()
	TWeakObjectPtr<AFGBuildable> SnappedObj = nullptr;
	
	AFINWirelessAccessPointHologram();
	
	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin AFGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void CheckValidPlacement() override;
	virtual void SetHologramLocationAndRotation(const FHitResult& HitResult) override;
	virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) override;
	// virtual void CheckValidFloor() override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	// End AFGBuildableHologram
};
