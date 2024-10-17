#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Processor/FINStateEEPROM_Legacy.h"
#include "FINItemStateEEPROMLua_Legacy.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKSLUA_API AFINStateEEPROMLua_Legacy : public AFINStateEEPROM_Legacy, public IFGLegacyItemStateActorInterface {
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, SaveGame)
	FString Code;

public:
	// Begin IFGLegacyItemStateActorInterface
	virtual FFGDynamicStruct ConvertToItemState(TSubclassOf<UFGItemDescriptor> itemDescriptor) const override {
		FFINItemStateEEPROMText state;
		state.Label = Label;
		state.Code = Code;
		return FFGDynamicStruct(state);
	}
	// End IFGLegacyItemStateActorInterface
};
