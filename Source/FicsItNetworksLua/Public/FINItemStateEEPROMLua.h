#pragma once

#include "CoreMinimal.h"
#include "FINComputerEEPROMDesc.h"
#include "FINItemStateEEPROM.h"
#include "FicsItKernel/Processor/FINStateEEPROM_Legacy.h"
#include "FINItemStateEEPROMLua.generated.h"

USTRUCT(BlueprintType)
struct FICSITNETWORKSLUA_API FFINItemStateEEPROMLua : public FFINItemStateEEPROM {
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString Code;
};

UCLASS(BlueprintType)
class FICSITNETWORKSLUA_API AFINStateEEPROMLua_Legacy : public AFINStateEEPROM_Legacy, public IFGLegacyItemStateActorInterface {
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadWrite, SaveGame)
	FString Code;

public:
	// Begin IFGLegacyItemStateActorInterface
	virtual FFGDynamicStruct ConvertToItemState(TSubclassOf<UFGItemDescriptor> itemDescriptor) const override;
	// End IFGLegacyItemStateActorInterface
};
