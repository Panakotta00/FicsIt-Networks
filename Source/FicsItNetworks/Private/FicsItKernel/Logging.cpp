#include "FicsItKernel/Logging.h"

#include "FGPlayerController.h"
#include "Computer/FINComputerRCO.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "Log"

FText FFINLogEntry::GetVerbosityAsText() const {
	switch (Verbosity) {
	case FIN_Log_Verbosity_Debug: return LOCTEXT("VerbosityDebug", "Debug");
	case FIN_Log_Verbosity_Info: return LOCTEXT("VerbosityInfo", "Info");
	case FIN_Log_Verbosity_Warning: return LOCTEXT("VerbosityWarning", "Warning");
	case FIN_Log_Verbosity_Error: return LOCTEXT("VerbosityError", "Error");
	case FIN_Log_Verbosity_Fatal: return LOCTEXT("VerbosityFatal", "Fatal");
	default: return FText();
	}
}

FString FFINLogEntry::ToClipboardText() const {
	FString TimestampText = Timestamp.ToString();
	FString VerbosityText = GetVerbosityAsText().ToString();
	return FString::Printf(TEXT("%s [%s] %s"), *TimestampText, *VerbosityText, *Content);
}

UFINLog::UFINLog() {
	SetIsReplicatedByDefault(true);
}

void UFINLog::BeginPlay() {
	Super::BeginPlay();

	if (!GetOwner()->HasAuthority()) {
		GetWorld()->GetFirstPlayerController<AFGPlayerController>()->GetRemoteCallObjectOfClass<UFINComputerRCO>()->LogRehandleAllEntries(this);
	}
}

void UFINLog::Tick() {
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	if (!LogEntriesToAdd.IsEmpty()) {
		TArray<FFINLogEntry> Chunk(LogEntriesToAdd.GetData(), FMath::Min(10, LogEntriesToAdd.Num()));
		LogEntriesToAdd.RemoveAt(0, Chunk.Num());
		Multicast_AddLogEntries(Chunk);
	} else if (bForceEntriesUpdate) {
		bForceEntriesUpdate = false;
		LogEntriesToAdd = LogEntries;
		LogEntries.Empty();
		Multicast_EmptyLog();
	}
}

void UFINLog::PushLogEntry(TEnumAsByte<EFINLogVerbosity> Verbosity, const FString& Content) {
	if (!this) return;
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	LogEntriesToAdd.Push(FFINLogEntry(FDateTime::UtcNow(), Verbosity, Content));
}

void UFINLog::EmptyLog() {
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	LogEntriesToAdd.Empty();
	LogEntries.Empty();
	bForceEntriesUpdate = true;
}

const TArray<FFINLogEntry>& UFINLog::GetLogEntries() {
	return LogEntries;
}

FString UFINLog::GetLogAsRichText() {
	FString Text;
	for (FFINLogEntry Entry : LogEntries) {
		FString TimestampText = Entry.Timestamp.ToString();
		FString VerbosityText = Entry.GetVerbosityAsText().ToString();
		Text.Append(FString::Printf(TEXT("<%s>%s [%s] %s</>\n"), *VerbosityText, *TimestampText, *VerbosityText, *Entry.Content));
	}
	return Text;
}

void UFINLog::RehandleAllEntries() {
	bForceEntriesUpdate = true;
}

void UFINLog::Multicast_EmptyLog_Implementation() {
	LogEntries.Empty();
	OnLogEntriesUpdated.Broadcast();
}

void UFINLog::Multicast_AddLogEntries_Implementation(const TArray<FFINLogEntry>& InLogEntries) {
	LogEntries.Append(InLogEntries);
	if (LogEntries.Num() > MaxLogEntries) LogEntries.RemoveAt(0, LogEntries.Num() - MaxLogEntries);
	OnLogEntriesUpdated.Broadcast();
}

void UFINLogLibrary::Log(TEnumAsByte<EFINLogVerbosity> Verbosity, FString Message, TEnumAsByte<EFINLogOptions> Options) {
	switch (Options) {
	case FIN_Log_Option_Where: {
		FString Where = FFINLogScope::Where();
		if (!Where.IsEmpty()) Where.AppendChar(L' ');
		FFINLogScope::GetLog()->PushLogEntry(Verbosity, Where.Append(Message));
		break;
	} default:
		FFINLogScope::GetLog()->PushLogEntry(Verbosity, *Message);
	}
}

UFINLog*& FFINLogScope::GetCurrentLog() {
	thread_local UFINLog* CurrentLog = nullptr;
	return CurrentLog;
}

FFINLogScope::FWhereFunction& FFINLogScope::GetCurrentWhereFunction() {
	thread_local FWhereFunction WhereFunction;
	return WhereFunction;
}

FFINLogScope::FStackFunction& FFINLogScope::GetCurrentStackFunction() {
	thread_local FStackFunction StackFunction;
	return StackFunction;
}
