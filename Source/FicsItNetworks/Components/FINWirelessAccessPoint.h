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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINWirelessAccessPointConnectionsUpdateDelegate);

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

	// Networking
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End Networking

	static float GetWirelessRange(AFGBuildableRadarTower* TargetTower);

	UFUNCTION(BlueprintCallable, Category="Network|Wireless")
	bool IsInWirelessRange(AFGBuildableRadarTower* Tower);

	/**
	 * Identifies the connections to other Wireless Access Points or Radar Towers
	 * with detailed information for each of them
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Wireless")
	TArray<UFINWirelessAccessPointConnection*> GetDisplayedWirelessConnections();

	/** Cached and replicated connections data. */
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_WirelessConnectionsData, BlueprintReadOnly, Category = "Network|Wireless")
	TArray<FFINWirelessAccessPointConnectionData> mAvailableWirelessConnectionsData;

	/** Locally displayed connections, calculated from mAvailableWirelessConnectionsData if on Guest MP. */
	UPROPERTY(BlueprintReadOnly, Category="Network|Wireless")
	TArray<UFINWirelessAccessPointConnection*> mDisplayedWirelessConnections;

	/** Updates (both on Host and Guest MP) the locally displayed connections. */
	void RefreshWirelessConnectionsData();

	UPROPERTY(BlueprintAssignable, Category = "Network|Wireless")
	FFINWirelessAccessPointConnectionsUpdateDelegate OnWirelessConnectionsDataUpdated;

	UFUNCTION()
	void OnRep_WirelessConnectionsData() {
		UE_LOG(LogFicsItNetworks, Log, TEXT("OnRep_WirelessConnectionsData received %d connections"), mAvailableWirelessConnectionsData.Num());
		mDisplayedWirelessConnections.Reset();
		for (const FFINWirelessAccessPointConnectionData& Data : mAvailableWirelessConnectionsData) {
			auto Connection = NewObject<UFINWirelessAccessPointConnection>();
			Connection->FromData(Data);
			mDisplayedWirelessConnections.Add(Connection);
		}

		OnWirelessConnectionsDataUpdated.Broadcast();
	}

	UPROPERTY()
	TArray<FGuid> HandledMessages;
	FCriticalSection HandleMessageMutex;

private:
	/**
	 * We store a cache of the connected access points (calculated by WirelessSubsystem) to
	 * avoid expensive calculations each tick.
	 * We don't care about replication since these are used only on the host.
	 */
	UPROPERTY()
	TArray<UFINWirelessAccessPointConnection*> mWirelessConnections;

	/**
	 * Verifies if this Access Point is ready to send/receive signals
	 */
	bool CanHandleMessages();
	
	bool HandleMessage(EFINWirelessDirection Direction, const FGuid& ID, const FGuid& Sender, const FGuid& Receiver, int Port, const TArray<FFINAnyNetworkValue>& Data);
};
