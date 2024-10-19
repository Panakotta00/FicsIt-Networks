#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "FINNetworkCable.generated.h"

class UFINNetworkConnectionComponent;

UCLASS()
class FICSITNETWORKSCIRCUIT_API AFINNetworkCable : public AFGBuildable {
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, Replicated, ReplicatedUsing=OnConnectorUpdate)
	UFINNetworkConnectionComponent* Connector1 = nullptr;

	UPROPERTY(SaveGame, Replicated, ReplicatedUsing=OnConnectorUpdate)
	UFINNetworkConnectionComponent* Connector2 = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class USplineMeshComponent* CableSpline = nullptr;
	
	AFINNetworkCable();
	~AFINNetworkCable();
	
	UPROPERTY(EditDefaultsOnly)
	float MaxCableSlack = 250;

	UPROPERTY(EditDefaultsOnly)
	float SlackLengthFactor = 1;

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
	void ReconstructCable();

	UFUNCTION()
	void OnConnectorUpdate();

	UFUNCTION(NetMulticast, Reliable)
	void ConnectConnectors();
};
