#pragma once

#include "CoreMinimal.h"
#include "FILLogEntry.h"
#include "FGSaveInterface.h"
#include "Components/ActorComponent.h"
#include "FILLogContainer.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINLogEntriesUpdatedDelegate);

UCLASS()
class FICSITLOGLIBRARY_API UFILLogContainer : public UActorComponent, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UFILLogContainer();

	// Begin UActorComponent
	virtual void BeginPlay() override;
	// End UActorComponent

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void PushLogEntry(TEnumAsByte<EFILLogVerbosity> Verbosity, const FString& Content);
	UFUNCTION(BlueprintCallable)
	void EmptyLog();

	UFUNCTION(BlueprintCallable)
	const TArray<FFILEntry>& GetLogEntries();

	UFUNCTION(BlueprintCallable)
	FString GetLogAsRichText();

	UFUNCTION()
	void RehandleAllEntries();

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddLogEntries(const TArray<FFILEntry>& InLogEntries);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EmptyLog();

public:
	UPROPERTY()
	int64 MaxLogEntries = 1000;

	UPROPERTY(BlueprintAssignable)
	FFINLogEntriesUpdatedDelegate OnLogEntriesUpdated;

	FScopeLock Lock() { return FScopeLock(&LogEntriesToAddMutex); }

private:
	UPROPERTY(SaveGame)
	TArray<FFILEntry> LogEntries;

	TArray<FFILEntry> LogEntriesToAdd;
	FCriticalSection LogEntriesToAddMutex;
	bool bForceEntriesUpdate = false;
};
