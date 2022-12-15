#pragma once

#include "Components/FINWirelessAccessPoint.h"
#include "FGRemoteCallObject.h"
#include "FINWirelessRCO.generated.h"

/**
 * RCO used to trigger updates on the server, in order to recalculate the currently displayed
 * radar towers & WAP in a single Access Point.
 * This way, whenever a client opens the Widget UI, the server will recalculate the towers and
 * send the updated ConnectionData to the client.
 */
UCLASS(Blueprintable)
class FICSITNETWORKS_API UFINWirelessRCO : public UFGRemoteCallObject {
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	bool bDummy = false;

	/**
	 * Triggers availble connections recalculation on server. These will be sent back to the client
	 * throught Replication.
	 */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="Network|Wireless|RCO")
	void SubscribeWirelessAccessPointConnections(AFINWirelessAccessPoint* AccessPoint, bool bSubscribe);
};
