#pragma once

#include "CoreMinimal.h"
#include "FINWirelessAccessPointActorRepresentation.h"
#include "Buildables/FGBuildableRadarTower.h"
#include "UObject/Object.h"
#include "FINWirelessAccessPointConnection.generated.h"

class AFINWirelessAccessPoint;

/**
 * Pairs of Wireless AP & Radar Towers, with signal strength and distance relative
 * to WAP where connection is analyzed.
 */
UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINWirelessAccessPointConnection : public UObject {
	GENERATED_BODY()

public:
	UFINWirelessAccessPointConnection() : RadarTower(nullptr), AccessPoint(nullptr), Distance(0.0f), IsConnected(false), IsInRange(false) {}
		
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AFGBuildableRadarTower> RadarTower;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AFINWirelessAccessPoint> AccessPoint;

	UPROPERTY(BlueprintReadOnly)
	float Distance;

	/** Identifies if Radar Tower is connected to a WAP */
	UPROPERTY(BlueprintReadOnly)
	bool IsConnected;

	/** Identifies if Radar Tower is within this tower communication range. */
	UPROPERTY(BlueprintReadOnly)
	bool IsInRange;

	/** If this a direct or repeated connection (with a Repeater Tower between source & target) */
	UPROPERTY(BlueprintReadOnly)
	bool IsRepeated;

	UPROPERTY(BlueprintReadOnly)
	UFINWirelessAccessPointActorRepresentation* ActorRepresentation = nullptr;

	/**
	 * Check if communication can be established through this connection from the source WAP.
	 */
	bool CanCommunicate() const {
		return IsConnected && IsInRange && !IsRepeated;
	}
};
