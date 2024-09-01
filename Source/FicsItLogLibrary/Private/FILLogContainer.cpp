#include "FILLogContainer.h"

#include "FGPlayerController.h"

UFILLogContainer::UFILLogContainer() {
	SetIsReplicatedByDefault(true);
}

void UFILLogContainer::BeginPlay() {
	Super::BeginPlay();

	if (!GetOwner()->HasAuthority()) {
		GetWorld()->GetFirstPlayerController<AFGPlayerController>()->GetRemoteCallObjectOfClass<UFINComputerRCO>()->LogRehandleAllEntries(this);
	}
}

void UFILLogContainer::Tick() {
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	if (!LogEntriesToAdd.IsEmpty()) {
		TArray<FFILLogEntry> Chunk(LogEntriesToAdd.GetData(), FMath::Min(10, LogEntriesToAdd.Num()));
		LogEntriesToAdd.RemoveAt(0, Chunk.Num());
		Multicast_AddLogEntries(Chunk);
	} else if (bForceEntriesUpdate) {
		bForceEntriesUpdate = false;
		LogEntriesToAdd = LogEntries;
		LogEntries.Empty();
		Multicast_EmptyLog();
	}
}

void UFILLogContainer::PushLogEntry(TEnumAsByte<EFILLogVerbosity> Verbosity, const FString& Content) {
	if (!this) return;
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	LogEntriesToAdd.Push(FFILLogEntry(FDateTime::UtcNow(), Verbosity, Content));
}

void UFILLogContainer::EmptyLog() {
	FScopeLock ScopeLock(&LogEntriesToAddMutex);
	LogEntriesToAdd.Empty();
	LogEntries.Empty();
	bForceEntriesUpdate = true;
}

const TArray<FFILLogEntry>& UFILLogContainer::GetLogEntries() {
	return LogEntries;
}

FString UFILLogContainer::GetLogAsRichText() {
	FString Text;
	for (FFILLogEntry Entry : LogEntries) {
		FString TimestampText = Entry.Timestamp.ToString();
		FString VerbosityText = Entry.GetVerbosityAsText().ToString();
		Text.Append(FString::Printf(TEXT("<%s>%s [%s] %s</>\n"), *VerbosityText, *TimestampText, *VerbosityText, *Entry.Content));
	}
	return Text;
}

void UFILLogContainer::RehandleAllEntries() {
	bForceEntriesUpdate = true;
}

void UFILLogContainer::Multicast_EmptyLog_Implementation() {
	LogEntries.Empty();
	OnLogEntriesUpdated.Broadcast();
}

void UFILLogContainer::Multicast_AddLogEntries_Implementation(const TArray<FFILLogEntry>& InLogEntries) {
	LogEntries.Append(InLogEntries);
	if (LogEntries.Num() > MaxLogEntries) LogEntries.RemoveAt(0, LogEntries.Num() - MaxLogEntries);
	OnLogEntriesUpdated.Broadcast();
}
