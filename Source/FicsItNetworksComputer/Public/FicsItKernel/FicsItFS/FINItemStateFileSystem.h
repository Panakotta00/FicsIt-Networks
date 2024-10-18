#pragma once

#include "CoreMinimal.h"
#include "FINLabelContainerInterface.h"
#include "FINItemStateFileSystem.generated.h"

USTRUCT(BlueprintType)
struct FICSITNETWORKSCOMPUTER_API FFINItemStateFileSystem : public FFINLabelContainerInterface {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid ID;

	UPROPERTY(SaveGame, EditDefaultsOnly)
	int32 Capacity = 0;

	UPROPERTY(SaveGame)
	FString Label;

	// Begin FFINLabelContainerInterface
	virtual FString GetLabel() const { return Label; }
	virtual void SetLabel(const FString& value) { Label = value; }
	// End FFINLabelContainerInterface

	bool Serialize(FStructuredArchive::FSlot Slot);
};

template<>
struct TStructOpsTypeTraits<FFINItemStateFileSystem> : TStructOpsTypeTraitsBase2<FFINItemStateFileSystem> {
	enum {
		WithStructuredSerializer = true,
	};
};
