#pragma once

#include "FINSignalData.h"
#include "FGSaveInterface.h"
#include "Subsystem/ModSubsystem.h"
#include "FINSignalSubsystem.generated.h"

USTRUCT()
struct FICSITNETWORKS_API FFINSignalListeners {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FFIRTrace> Listeners;

	void AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const;
};

template<>
struct TStructOpsTypeTraits<FFINSignalListeners> : TStructOpsTypeTraitsBase2<FFINSignalListeners> {
	enum {
		WithAddStructReferencedObjects = true,
	};
};

UCLASS(BlueprintType)
class FICSITNETWORKS_API AFINSignalSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
private:
	/**
	 * Map of sender objects to array of receiver traces, traces point from sender to receiver
	 */
	UPROPERTY(SaveGame)
	TMap<UObject*, FFINSignalListeners> Listeners;
	
public:
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	// End IFGSaveInterface

	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& ReferenceCollector) {
		Super::AddReferencedObjects(InThis, ReferenceCollector);
		AFINSignalSubsystem* SigSubSys = Cast<AFINSignalSubsystem>(InThis);
		for (TPair<UObject*, FFINSignalListeners>& Listener : SigSubSys->Listeners) Listener.Value.AddStructReferencedObjects(ReferenceCollector);
	}

	/**
	 * Removes all Listeners and sender that don't exist anymore
	 */
	void Cleanup();

	/**
	* Gets the loaded signal subsystem in the given world.
	*
	* @param[in]	WorldContext	the world context from were to load the signal subsystem.
	*/
	UFUNCTION(BlueprintCallable, Category = "Network|Signals", meta = (WorldContext = "WorldContext"))
    static AFINSignalSubsystem* GetSignalSubsystem(UObject* WorldContext);

	/**
	 * Distributes the given signal to all listeners listening to the given object
	 */
	void BroadcastSignal(UObject* Sender, const FFINSignalData& Signal);

	/**
	 * Adds the given listener to the listener list of the given sender
	 * the Receiver Trace should point from sender to the receiver
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Signals")
	void Listen(UObject* Sender, const FFIRTrace& Receiver);

	/**
	 * Removes the given listener from the listener list of the given sender
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Signals")
	void Ignore(UObject* Sender, UObject* Receiver);

	/**
	 * Removes the given listener form all senders
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Signals")
	void IgnoreAll(UObject* Receiver);

	/**
	 * Returns all the objects this object listens to
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Signals")
	TArray<UObject*> GetListening(UObject* Reciever);
};
