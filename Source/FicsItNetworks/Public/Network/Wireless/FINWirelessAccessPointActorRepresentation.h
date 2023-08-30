#pragma once

#include "FGActorRepresentation.h"
#include "FINWirelessAccessPointActorRepresentation.generated.h"

class UFINWirelessAccessPointConnection;

/**
 * ActorRepresentation for a Wireless Access Point. This is used to show the Access Point connection status relative
 * to the currently selected access point in the UI map.
 *
 * Note: this ActorRepresentation is _not_ used in the player map, just in the map inside the WAP widget gui.
 */
UCLASS()
class FICSITNETWORKS_API UFINWirelessAccessPointActorRepresentation : public UFGActorRepresentation {
	GENERATED_BODY()

public:
	UFINWirelessAccessPointActorRepresentation();
	
	inline static const FLinearColor Green = FLinearColor::FromSRGBColor(FColor(128, 177, 57));
	inline static const FLinearColor Orange = FLinearColor::FromSRGBColor(FColor(242, 100, 24));
	inline static const FLinearColor Red = FLinearColor::FromSRGBColor(FColor(210, 52, 48));
	
	void Setup(UFINWirelessAccessPointConnection* Connection);

	// Begin UFGActorRepresentation
	virtual bool IsSupportedForNetworking() const override { return false; }
	virtual void SetupActorRepresentation( AActor* realActor, bool isLocal, float lifeSpan = 0.0f ) override;
	virtual FVector GetActorLocation() const override;
	virtual void TrySetupDestroyTimer( float lifeSpan ) override;
	virtual bool CanBeHighlighted() const override;
	// End UFGActorRepresentation
private:
	UPROPERTY()
	FVector mRealActorLocation;
};
