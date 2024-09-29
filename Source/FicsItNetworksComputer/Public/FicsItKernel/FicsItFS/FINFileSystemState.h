#pragma once

#include "CoreMinimal.h"
#include "FGLegacyItemStateActorInterface.h"
#include "FINLabelContainerInterface.h"
#include "Library/Device.h"
#include "FINFileSystemState.generated.h"

USTRUCT(BlueprintType)
struct FFINFileSystemState : public FFINLabelContainerInterface {
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
struct TStructOpsTypeTraits<FFINFileSystemState> : TStructOpsTypeTraitsBase2<FFINFileSystemState> {
	enum {
		WithStructuredSerializer = true,
	};
};
