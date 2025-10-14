#pragma once
#include "CoreMinimal.h"

#include "Hologram/FGGenericBuildableHologram.h"

#include "FINCabinetPanelHolo.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINCabinetPanelHolo : public AFGGenericBuildableHologram {
    GENERATED_BODY()
    
    public:
        // Sets default values for this actor's properties
        AFINCabinetPanelHolo();

	protected:
		virtual bool IsHologramIdenticalToActor(AActor* actor, const FTransform& hologramLocationOffset) const override;
};


