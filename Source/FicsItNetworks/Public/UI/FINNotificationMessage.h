#pragma once

#include "CoreMinimal.h"
#include "UI/Message/FGMessageBase.h"
#include "FINNotificationMessage.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINNotificationMessage : public UFGMessageBase {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, meta=(ExposeOnSpawn))
	FText NotificationText;
};