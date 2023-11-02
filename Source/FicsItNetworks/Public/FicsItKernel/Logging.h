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
struct FICSITNETWORKS_API FFINLogEntry {
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
class FICSITNETWORKS_API UFINLog : public UObject, public IFGSaveInterface {
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

UENUM()
enum EFINLogOptions {
	FIN_Log_Option_None,
	FIN_Log_Option_Where,
	FIN_Log_Option_Stack,
};

UCLASS()
class FICSITNETWORKS_API UFINLogLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static void Log(TEnumAsByte<EFINLogVerbosity> Verbosity, FString Message, TEnumAsByte<EFINLogOptions> Options = FIN_Log_Option_Where);
};

class FICSITNETWORKS_API FFINLogScope {
public:
	DECLARE_DELEGATE_RetVal(FString, FWhereFunction);
	DECLARE_DELEGATE_RetVal(FString, FStackFunction);
	
	FFINLogScope(UFINLog* Log, FWhereFunction WhereFunction = FWhereFunction(), FStackFunction StackFunction = FStackFunction()) {
		PreviousLog = GetCurrentLog();
		if (Log) GetCurrentLog() = Log;

		if (WhereFunction.IsBound()) {
			PreviousWhereFunction = GetCurrentWhereFunction();
			GetCurrentWhereFunction() = WhereFunction;
		}

		if (StackFunction.IsBound()) {
			PreviousStackFunction = GetCurrentStackFunction();
			GetCurrentStackFunction() = WhereFunction;
		}
	}

	~FFINLogScope() {
		GetCurrentLog() = PreviousLog;

		if (PreviousWhereFunction.IsSet()) {
			GetCurrentWhereFunction() = PreviousWhereFunction.GetValue();
		}

		if (PreviousStackFunction.IsSet()) {
			GetCurrentStackFunction() = PreviousStackFunction.GetValue();
		}
	}

	FORCEINLINE static UFINLog* GetLog() { return GetCurrentLog(); }
	FORCEINLINE static FString Where() {
		FWhereFunction& Func = GetCurrentWhereFunction();
		if (Func.IsBound()) return Func.Execute();
		return FString();
	}
	FORCEINLINE static FString Stack() {
		FStackFunction& Func = GetCurrentStackFunction();
		if (Func.IsBound()) return Func.Execute();
		return FString();
	}
	
private:
	static UFINLog*& GetCurrentLog();
	static FWhereFunction& GetCurrentWhereFunction();
	static FStackFunction& GetCurrentStackFunction();

	UFINLog* PreviousLog;
	TOptional<FWhereFunction> PreviousWhereFunction;
	TOptional<FStackFunction> PreviousStackFunction;
};
