#include "FicsItKernel/Logging.h"

#define LOCTEXT_NAMESPACE "Log"

void UFINLog::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	DOREPLIFETIME(UFINLog, LogEntries);
}

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

void UFINLog::Tick() {
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	if (!LogEntriesToAdd.IsEmpty()) {
		LogEntries.Append(LogEntriesToAdd);
		if (LogEntries.Num() > MaxLogEntries) LogEntries.RemoveAt(0, LogEntries.Num() - MaxLogEntries);
		LogEntriesToAdd.Empty();
		OnLogEntriesUpdated.Broadcast();
	} else if (bForceEntriesUpdate) {
		bForceEntriesUpdate = false;
		OnLogEntriesUpdated.Broadcast();
	}
}

void UFINLog::PushLogEntry(TEnumAsByte<EFINLogVerbosity> Verbosity, const FString& Content) {
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	LogEntriesToAdd.Push(FFINLogEntry(FDateTime::UtcNow(), Verbosity, Content));
}

void UFINLog::EmptyLog() {
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	LogEntriesToAdd.Empty();
	LogEntries.Empty();
	bForceEntriesUpdate = true;
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

void UFINLog::OnRep_LogEntries() {
	OnLogEntriesUpdated.Broadcast();
}
