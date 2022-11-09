#pragma once

#include "CoreMinimal.h"
#include "FINWirelessAccessPointConnection.h"
#include "Buildables/FGBuildableRadarTower.h"
#include "FicsItNetworks/FicsItNetworksModule.h"
#include "Subsystem/ModSubsystem.h"
#include "FINWirelessSubsystem.generated.h"

class AFINWirelessAccessPoint;

/**
 * Subsystem to recalculate wireless network topology on changes
 */
UCLASS()
class FICSITNETWORKS_API AFINWirelessSubsystem : public AModSubsystem {
	GENERATED_BODY()

public:
	AFINWirelessSubsystem();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	bool bDummy;

	UFUNCTION(BlueprintCallable, Category = "Network|Wireless", meta = (WorldContext = "WorldContext"))
	static AFINWirelessSubsystem* Get(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Network|Wireless")
	TArray<AFINWirelessAccessPoint*> GetAccessPoints();

	UFUNCTION(BlueprintCallable, Category = "Network|Wireless")
	void RecalculateWirelessConnections();

	/**
	 * Identifies the connections to other Wireless Access Points or Radar Towers
	 * with detailed information for each of them
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Wireless")
	TArray<UFINWirelessAccessPointConnection*> GetAvailableConnections(AFINWirelessAccessPoint* CurrentAccessPoint);

	UPROPERTY(BlueprintReadOnly, Category = "Network|Wireless")
	TArray<AActor*> CachedAccessPoints;

	UPROPERTY(BlueprintReadOnly, Category = "Network|Wireless")
	TArray<AActor*> CachedRadarTowers;
	
protected:
	virtual void BeginPlay() override;
	void CacheTowersAndAccessPoints();
};
