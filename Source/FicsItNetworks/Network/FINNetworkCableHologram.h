#pragma once

#include "CoreMinimal.h"
#include "FGBuildableHologram.h"
#include "FGBuildableFactory.h"
#include "FINNetworkConnector.h"
#include "Components/SplineMeshComponent.h"
#include "FINNetworkCableHologram.generated.h"

USTRUCT()
struct FICSITNETWORKS_API FFINSnappedInfo {
	GENERATED_BODY()

public:
	bool v = false;
	bool isConnector = false;
	void* ptr = nullptr;
	AActor* actor = nullptr;
	FVector pos;
	FQuat rot;

	inline UFINNetworkConnector* c() {
		return (UFINNetworkConnector*)ptr;
	}

	inline AFGBuildableFactory* f() {
		return (AFGBuildableFactory*)ptr;
	}
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

	UPROPERTY()
	FFINSnappedInfo Snapped;

	UPROPERTY()
	FFINSnappedInfo OldSnapped;

	UPROPERTY()
	FFINSnappedInfo From;

	// Begin FGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual AActor* Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool IsValidHitResult(const FHitResult& hit) const override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hit) override;
	virtual void OnInvalidHitResult() override;
	virtual bool IsChanged() const override;
	virtual USceneComponent* SetupComponent(USceneComponent* attachParent, UActorComponent* templateComponent, const FName& componentName) override;
	// End FGBuildableHologram

	AFINNetworkCableHologram();
	~AFINNetworkCableHologram();

	void onBeginSnap(FFINSnappedInfo a, bool isValid);
	void onEndSnap(FFINSnappedInfo a);
	void updateMeshes();
	UFINNetworkConnector* setupSnapped(FFINSnappedInfo s);

	bool isSnappedValid();
	bool isValid();
};