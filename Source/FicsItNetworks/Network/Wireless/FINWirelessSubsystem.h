// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

	UFUNCTION(BlueprintCallable, Category = "Network|Wireless", meta = (WorldContext = "WorldContext"))
	static AFINWirelessSubsystem* Get(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Network|Wireless")
	TArray<AFINWirelessAccessPoint*> GetAccessPoints();

	UFUNCTION(BlueprintCallable, Category = "Network|Wireless")
	void RecalculateWirelessConnections();

protected:
	virtual void BeginPlay() override;
};
