#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "FINNetworkCable.generated.h"

class UFINNetworkConnectionComponent;

UCLASS()
class FICSITNETWORKSCIRCUIT_API AFINNetworkCable : public AFGBuildable {
	GENERATED_BODY()
public:
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
	void SetConnection(UFINNetworkConnectionComponent* InConnector1, UFINNetworkConnectionComponent* InConnector2);
	TTuple<UFINNetworkConnectionComponent*, UFINNetworkConnectionComponent*> GetConnections() const;

	UFUNCTION()
	void ReconstructCable();

	UFUNCTION()
	void OnConnectorUpdate();

	UFUNCTION(NetMulticast, Reliable)
	void ConnectConnectors();

	FORCEINLINE UFINNetworkConnectionComponent* GetOtherConnector(UFINNetworkConnectionComponent* Connection) {
		if (Connection == Connector1) return Connector2;
		if (Connection == Connector2) return Connector1;
		return nullptr;
	}

private:
	UPROPERTY(SaveGame, Replicated, ReplicatedUsing=OnConnectorUpdate)
	UFINNetworkConnectionComponent* Connector1 = nullptr;

	UPROPERTY(SaveGame, Replicated, ReplicatedUsing=OnConnectorUpdate)
	UFINNetworkConnectionComponent* Connector2 = nullptr;
};
