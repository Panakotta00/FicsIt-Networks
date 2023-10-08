#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "Logging.generated.h"

UENUM()
enum EFINLogVerbosity {
	FIN_Log_Verbosity_Debug,
	FIN_Log_Verbosity_Info,
	FIN_Log_Verbosity_Warning,
	FIN_Log_Verbosity_Error,
	FIN_Log_Verbosity_Fatal,
};

USTRUCT(BlueprintType)
struct FFINLogEntry {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FDateTime Timestamp;
	
	UPROPERTY(SaveGame)
	TEnumAsByte<EFINLogVerbosity> Verbosity;

	UPROPERTY(SaveGame)
	FString Content;

	FFINLogEntry() = default;
	FFINLogEntry(FDateTime Timestamp, TEnumAsByte<EFINLogVerbosity> Verbosity, const FString& Content) : Timestamp(Timestamp), Verbosity(Verbosity), Content(Content) {}

	FText GetVerbosityAsText() const;
	FString ToClipboardText() const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINLogEntriesUpdatedDelegate);

UCLASS()
class UFINLog : public UObject, public IFGSaveInterface {
	GENERATED_BODY()
public:
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface
	
	UFUNCTION()
	void Tick();
	
	UFUNCTION(BlueprintCallable)
	void PushLogEntry(TEnumAsByte<EFINLogVerbosity> Verbosity, const FString& Content);
	UFUNCTION(BlueprintCallable)
	void EmptyLog();

	UFUNCTION(BlueprintCallable)
	const TArray<FFINLogEntry>& GetLogEntries() { return LogEntries; }

	UFUNCTION(BlueprintCallable)
	FString GetLogAsRichText();
	
private:
	UFUNCTION()
	void OnRep_LogEntries();
	
public:
	UPROPERTY()
	int64 MaxLogEntries = 1000;
	
	UPROPERTY(BlueprintAssignable)
	FFINLogEntriesUpdatedDelegate OnLogEntriesUpdated;

private:
	UPROPERTY(SaveGame, ReplicatedUsing=OnRep_LogEntries)
	TArray<FFINLogEntry> LogEntries;

	TArray<FFINLogEntry> LogEntriesToAdd;
	FCriticalSection LogEntriesToAddMutex;
	bool bForceEntriesUpdate = false;
};
