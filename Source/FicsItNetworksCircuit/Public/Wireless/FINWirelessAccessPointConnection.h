#pragma once

#include "CoreMinimal.h"
#include "FINWirelessAccessPointConnectionData.h"
#include "FINWirelessAccessPointConnection.generated.h"

class AFINWirelessAccessPoint;
class AFGBuildableRadarTower;
class UFINWirelessAccessPointActorRepresentation;

/**
 * Pairs of Wireless AP & Radar Towers, with signal strength and distance relative
 * to WAP where connection is analyzed.
 */
UCLASS(BlueprintType)
class FICSITNETWORKSCIRCUIT_API UFINWirelessAccessPointConnection : public UObject {
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
	bool CanCommunicate() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Network|Wireless")
	FText GetRepresentationText();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Network|Wireless")
	FVector GetRepresentationLocation() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Network|Wireless")
	bool HasPower() const;

	void FromData(const FFINWirelessAccessPointConnectionData& NewData);
	void SetupActorRepresentation();
};
