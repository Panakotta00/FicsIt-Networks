#pragma once

#include "CoreMinimal.h"
#include "FGBuildable.h"
#include "Components/SplineMeshComponent.h"
#include "FINNetworkCable.generated.h"

class UFINNetworkConnectionComponent;

UCLASS()
class FICSITNETWORKS_API AFINNetworkCable : public AFGBuildable {
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, Replicated, ReplicatedUsing=OnConnectorUpdate)
	UFINNetworkConnectionComponent* Connector1 = nullptr;

	UPROPERTY(SaveGame, Replicated, ReplicatedUsing=OnConnectorUpdate)
	UFINNetworkConnectionComponent* Connector2 = nullptr;

	UPROPERTY(EditDefaultsOnly)
	USplineMeshComponent* CableSpline = nullptr;

	AFINNetworkCable();
	~AFINNetworkCable();

	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type reason) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	// End IFGSaveInterface

	// Begin IFGDismantleInterface
	virtual int32 GetDismantleRefundReturnsMultiplier() const;
	// End IFGDismantleInterface

	UFUNCTION()
	void OnConnectorUpdate();
};
