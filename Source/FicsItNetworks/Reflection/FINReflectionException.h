#pragma once

#include "FINBase.h"
#include "FicsItNetworks/Utils/FINException.h"
#include "FINReflectionException.generated.h"

USTRUCT()
struct FICSITNETWORKS_API FFINReflectionException : public FFINException {
	GENERATED_BODY()

	UPROPERTY()
	UFINBase* Context = nullptr;

	FFINReflectionException() = default;
	FFINReflectionException(UFINBase* Context, const FString& Message) : FFINException(Message), Context(Context) {}
};
