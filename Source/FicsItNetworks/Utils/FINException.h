#pragma once

#include "CoreMinimal.h"
#include "FINException.generated.h"

USTRUCT()
struct FFINException {
	GENERATED_BODY()
	
	FString Message;

	FFINException() = default;
	FFINException(const FString& Message) : Message(Message) {}
	virtual ~FFINException() = default;

	virtual FString GetMessage() const { return Message; }
};
