#pragma once

#include "CoreMinimal.h"
#include "FIRException.generated.h"

class UFIRBase;

USTRUCT()
struct FICSITREFLECTION_API FFIRException {
	GENERATED_BODY()

	UPROPERTY()
	FString Message;

	FFIRException() = default;
	FFIRException(const FString& Message) : Message(Message) {}
	virtual ~FFIRException() = default;

	virtual FString GetMessage() const { return Message; }
};

USTRUCT()
struct FICSITREFLECTION_API FFIRReflectionException : public FFIRException {
	GENERATED_BODY()

	UPROPERTY()
	UFIRBase* Context = nullptr;

	FFIRReflectionException() = default;
	FFIRReflectionException(UFIRBase* Context, const FString& Message) : FFIRException(Message), Context(Context) {}
};
