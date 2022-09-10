#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableRadarTower.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
#include "FicsItNetworks/Network/Wireless/FINWirelessAccessPointConnection.h"
#include "FicsItNetworks/Network/Wireless/FINWirelessSubsystem.h"
#include "FINWirelessAccessPoint.generated.h"

UENUM(BlueprintType)
enum class EFINWirelessDirection : uint8 {
	FromWireless,
	FromCircuit
};

/**
 * The Wireless Access Point allows to connect an existing Network Circuit to a
 * Radar Tower, which sends/receives signals from other towers.
 */
UCLASS()
class FICSITNETWORKS_API AFINWirelessAccessPoint : public AFGBuildable
{
	friend class AFINWirelessSubsystem;
	
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, Replicated, BlueprintReadOnly)
	AFGBuildableRadarTower* AttachedTower = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector1;
	
	AFINWirelessAccessPoint();
	~AFINWirelessAccessPoint();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	static float GetWirelessRange(AFGBuildableRadarTower* TargetTower);

	UFUNCTION(BlueprintCallable, Category="Network|Wireless")
	bool IsInWirelessRange(AFGBuildableRadarTower* Tower);

	/**
	 * Identifies the connections to other Wireless Access Points or Radar Towers
	 * with detailed information for each of them
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Wireless")
	TArray<UFINWirelessAccessPointConnection*> GetAvailableWirelessConnections();

	UPROPERTY()
	TArray<FGuid> HandledMessages;
	FCriticalSection HandleMessageMutex;

private:
	/**
	 * We store a cache of the connected access points (calculated by WirelessSubsystem) to
	 * avoid expensive calculations each tick
	 */
	UPROPERTY()
	TArray<UFINWirelessAccessPointConnection*> mConnectedAccessPoints;
	
	bool HandleMessage(EFINWirelessDirection Direction, const FGuid& ID, const FGuid& Sender, const FGuid& Receiver, int Port, const TArray<FFINAnyNetworkValue>& Data);
};
