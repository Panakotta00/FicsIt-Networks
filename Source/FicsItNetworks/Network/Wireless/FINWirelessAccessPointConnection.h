#pragma once

#include "CoreMinimal.h"
#include "FINWirelessAccessPointActorRepresentation.h"
#include "FINWirelessAccessPointConnectionData.h"
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
	UFINWirelessAccessPointConnection() : RadarTower(nullptr), AccessPoint(nullptr) {}
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AFGBuildableRadarTower> RadarTower;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AFINWirelessAccessPoint> AccessPoint;

	// Conversion
	UPROPERTY(BlueprintReadOnly)
	FFINWirelessAccessPointConnectionData Data;

	UPROPERTY(BlueprintReadOnly)
	UFINWirelessAccessPointActorRepresentation* ActorRepresentation = nullptr;

	/**
	 * Check if communication can be established through this connection from the source WAP.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Network|Wireless")
	bool CanCommunicate() const {
		return Data.IsConnected && Data.IsInRange && !Data.IsRepeated && !Data.IsSelf;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Network|Wireless")
	FText GetRepresentationText() {
		return RadarTower == nullptr ? Data.RepresentationText : RadarTower->GetRepresentationText();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Network|Wireless")
	FVector GetRepresentationLocation() const {
		return RadarTower == nullptr ? Data.RepresentationLocation : RadarTower->GetActorLocation();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Network|Wireless")
	bool HasPower() const {
		return RadarTower == nullptr ? Data.RadarTowerHasPower : RadarTower->HasPower();
	}
	
	void FromData(const FFINWirelessAccessPointConnectionData& NewData);
	void SetupActorRepresentation();
};
