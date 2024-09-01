#pragma once

#include "CoreMinimal.h"
#include "FILLogEntry.generated.h"

UENUM()
enum EFILLogVerbosity {
	FIL_Verbosity_Debug,
	FIL_Verbosity_Info,
	FIL_Verbosity_Warning,
	FIL_Verbosity_Error,
	FIL_Verbosity_Fatal,
	FIL_Verbosity_Max = FIL_Verbosity_Fatal,
};

USTRUCT(BlueprintType)
struct FICSITLOGLIBRARY_API FFILLogEntry {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FDateTime Timestamp;

	UPROPERTY(SaveGame)
	TEnumAsByte<EFILLogVerbosity> Verbosity;

	UPROPERTY(SaveGame)
	FString Content;

	FFILLogEntry() = default;
	FFILLogEntry(FDateTime Timestamp, TEnumAsByte<EFILLogVerbosity> Verbosity, const FString& Content) : Timestamp(Timestamp), Verbosity(Verbosity), Content(Content) {}

	FText GetVerbosityAsText() const;
	FString ToClipboardText() const;
};
