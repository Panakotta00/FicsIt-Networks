#pragma once

#include "FGCentralStorageSubsystem.h"
#include "FicsItReflection.h"
#include "FIRHookSubsystem.h"
#include "Reflection/FIRClass.h"
#include "Reflection/Source/FIRSourceStatic.h"

#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionComponent.h"
#include "FGPowerCircuit.h"
#include "FGTrain.h"
#include "Buildables/FGBuildableRailroadSignal.h"
#include "Buildables/FGPipeHyperStart.h"
#include "Patching/NativeHookManager.h"
#include "FGCharacterMovementComponent.h"
#include "FGCharacterPlayer.h"
#include "Buildables/FGBuildableRailroadStation.h"

#include "FIRSourceStaticHooks.generated.h"

UCLASS()
class FICSITREFLECTION_API UFIRStaticHook : public UFIRHook {
	GENERATED_BODY()

protected:
	UPROPERTY()
	UFIRSignal* Signal = nullptr;

protected:
	UPROPERTY()
	UObject* Sender = nullptr;

protected:
	void Send(const TArray<FFIRAnyValue>& Values) {
		Signal->Trigger(Sender, Values);
	}

public:
	void Register(UObject* sender) override {
		Super::Register(sender);

		Sender = sender;
	}
};

UCLASS()
class FICSITREFLECTION_API UFIRFunctionHook : public UFIRHook {
	GENERATED_BODY()

private:
	UPROPERTY()
	UObject* Sender;

	bool bIsRegistered;

	UPROPERTY()
	TMap<FString, UFIRSignal*> Signals;

protected:
	UPROPERTY()
	TSet<TWeakObjectPtr<UObject>> Senders;
	FCriticalSection Mutex;

	bool IsSender(UObject* Obj) {
		FScopeLock Lock(&Mutex);
		return Senders.Contains(Obj);
	}

	void Send(UObject* Obj, const FString& SignalName, const TArray<FFIRAnyValue>& Data) {
		UFIRSignal** Signal = Signals.Find(SignalName);
		if (!Signal) {
			UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(Obj->GetClass());
			UFIRSignal* NewSignal = Class->FindFIRSignal(SignalName);
			if (!NewSignal) UE_LOG(LogFicsItReflection, Error, TEXT("Signal with name '%s' not found for object '%s' of FIRClass '%s'"), *SignalName, *Obj->GetName(), *Class->GetInternalName());
			Signal = &Signals.Add(SignalName, NewSignal);
		}
		if (Signal) (*Signal)->Trigger(Obj, Data);
	}

	virtual void RegisterFuncHook() {}

	virtual UFIRFunctionHook* Self() { return nullptr; }

public:
	void Register(UObject* sender) override {
		Super::Register(sender);

		FScopeLock Lock(&Self()->Mutex);
    	Self()->Senders.Add(Sender = sender);

		if (!Self()->bIsRegistered) {
			Self()->bIsRegistered = true;
			Self()->RegisterFuncHook();
		}
    }

	void Unregister() override {
		FScopeLock Lock(&Self()->Mutex);
    	Self()->Senders.Remove(Sender);
    }
};

UCLASS()
class FICSITREFLECTION_API UFIRMultiFunctionHook : public UFIRHook {
	GENERATED_BODY()

private:
	UPROPERTY()
	UObject* Sender;

	bool bIsRegistered;

	UPROPERTY()
	TMap<FString, UFIRSignal*> Signals;

protected:
	UPROPERTY()
	TSet<TWeakObjectPtr<UObject>> Senders;
	FCriticalSection Mutex;

	bool IsSender(UObject* Obj) {
		FScopeLock Lock(&Mutex);
		return Senders.Contains(Obj);
	}

	void Send(UObject* Obj, const FString& SignalName, const TArray<FFIRAnyValue>& Data) {
		UFIRSignal* Signal;
		if (Signals.Contains(SignalName)) {
			Signal = Signals[SignalName];
		}else{
			UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(Obj->GetClass());
			Signal = Class->FindFIRSignal(SignalName);
			if (!Signal) UE_LOG(LogFicsItReflection, Error, TEXT("Signal with name '%s' not found for object '%s' of FIRClass '%s'"), *SignalName, *Obj->GetName(), *Class->GetInternalName());
			Signals.Add(SignalName, Signal);
		}
		if (Signal) Signal->Trigger(Obj, Data);
	}

	virtual void RegisterFuncHook() {}

	virtual UFIRMultiFunctionHook* Self() { return nullptr; }

public:
	void Register(UObject* sender) override {
		Super::Register(sender);

		FScopeLock Lock(&Self()->Mutex);
    	Self()->Senders.Add(Sender = sender);

		if (!Self()->bIsRegistered) {
			Self()->bIsRegistered = true;
			Self()->RegisterFuncHook();
		}
    }

	void Unregister() override {
		FScopeLock Lock(&Self()->Mutex);
    	Self()->Senders.Remove(Sender);
    }
};

UCLASS()
class UFIRBuildableHook : public UFIRStaticHook {
	GENERATED_BODY()
private:
	FDelegateHandle Handle;

public:
	UFUNCTION()
	void ProductionStateChanged(EProductionStatus status) {
		Send({(int64)status});
	}

	void Register(UObject* sender) override {
		Super::Register(sender);

		UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(Sender->GetClass());
		Signal = Class->FindFIRSignal(TEXT("ProductionChanged"));

		Handle = Cast<AFGBuildable>(sender)->mOnProductionStatusChanged.AddUObject(this, &UFIRBuildableHook::ProductionStateChanged);
	}

	void Unregister() override {
		Cast<AFGBuildable>(Sender)->mOnProductionStatusChanged.Remove(Handle);
	}
};

UCLASS()
class UFIRRailroadTrackHook : public UFIRFunctionHook {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIRSignal* VehicleEnterSignal;

	UPROPERTY()
	UFIRSignal* VehicleExitSignal;

protected:
	static UFIRRailroadTrackHook* StaticSelf() {
		static UFIRRailroadTrackHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFIRRailroadTrackHook*>(GetDefault<UFIRRailroadTrackHook>());
		return Hook;
	}

	// Begin UFIRFunctionHook
	virtual UFIRFunctionHook* Self() override {
		return StaticSelf();
	}
	// End UFIRFunctionHook

private:
	static void VehicleEnter(AFGBuildableRailroadTrack* Track, AFGRailroadVehicle* Vehicle) {
		StaticSelf()->Send(Track, TEXT("VehicleEnter"), {(FIRTrace)Vehicle});
	}

	static void VehicleExit(AFGBuildableRailroadTrack* Track, AFGRailroadVehicle* Vehicle) {
		StaticSelf()->Send(Track, TEXT("VehicleExit"), {(FIRTrace)Vehicle});
	}

public:
	void RegisterFuncHook() override {
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableRailroadTrack::OnVehicleEntered, (void*)GetDefault<AFGBuildableRailroadTrack>(), &VehicleEnter);
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableRailroadTrack::OnVehicleExited, (void*)GetDefault<AFGBuildableRailroadTrack>(), &VehicleExit);
	}
};

UCLASS()
class UFIRTrainHook : public UFIRStaticHook {
	GENERATED_BODY()

public:
	UFUNCTION()
	void SelfDrvingUpdate(bool enabled) {
		Send({enabled});
	}

	void Register(UObject* sender) override {
		Super::Register(sender);

		UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(Sender->GetClass());
		Signal = Class->FindFIRSignal(TEXT("SelfDrvingUpdate"));

		Cast<AFGTrain>(sender)->mOnSelfDrivingChanged.AddDynamic(this, &UFIRTrainHook::SelfDrvingUpdate);
	}

	void Unregister() override {
		Cast<AFGTrain>(Sender)->mOnSelfDrivingChanged.RemoveDynamic(this, &UFIRTrainHook::SelfDrvingUpdate);
	}
};

UCLASS()
class UFIRRailroadStationHook : public UFIRFunctionHook {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIRSignal* VehicleEnterSignal;

	UPROPERTY()
	UFIRSignal* VehicleExitSignal;

protected:
	static UFIRRailroadStationHook* StaticSelf() {
		static UFIRRailroadStationHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFIRRailroadStationHook*>(GetDefault<UFIRRailroadStationHook>());
		return Hook;
	}

	// Begin UFIRFunctionHook
	virtual UFIRFunctionHook* Self() override {
		return StaticSelf();
	}
	// End UFIRFunctionHook

private:
	static void StartDocking(bool RetVal, AFGBuildableRailroadStation* Self, AFGLocomotive* Locomotive, float Offset) {
		StaticSelf()->Send(Self, TEXT("StartDocking"), {RetVal, (FIRTrace)(UObject*)Locomotive, Offset});
	}

	static void FinishDocking(AFGBuildableRailroadStation* Self) {
		StaticSelf()->Send(Self, TEXT("FinishDocking"), {});
	}

	static void CancelDocking(AFGBuildableRailroadStation* Self) {
		StaticSelf()->Send(Self, TEXT("CancelDocking"), {});
	}

public:
	void RegisterFuncHook() override {
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableRailroadStation::StartDocking, GetDefault<AFGBuildableRailroadStation>(), &UFIRRailroadStationHook::StartDocking);
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableRailroadStation::FinishDockingSequence, GetDefault<AFGBuildableRailroadStation>(), &UFIRRailroadStationHook::FinishDocking);
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableRailroadStation::CancelDockingSequence, GetDefault<AFGBuildableRailroadStation>(), &UFIRRailroadStationHook::CancelDocking);
	}
};

UCLASS()
class UFIRRailroadSignalHook : public UFIRStaticHook {
	GENERATED_BODY()

	UPROPERTY()
	UFIRSignal* ValidationChangedSignal;

public:
	UFUNCTION()
	void AspectChanged(ERailroadSignalAspect Aspect) {
		Send({(int64)Aspect});
	}

	UFUNCTION()
	void ValidationChanged(ERailroadBlockValidation Validation) {
		ValidationChangedSignal->Trigger(Sender, {(int64)Validation});
	}

	void Register(UObject* sender) override {
		Super::Register(sender);

		UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(Sender->GetClass());
		Signal = Class->FindFIRSignal(TEXT("AspectChanged"));

		ValidationChangedSignal = Class->FindFIRSignal(TEXT("ValidationChanged"));

		Cast<AFGBuildableRailroadSignal>(sender)->mOnAspectChangedDelegate.AddDynamic(this, &UFIRRailroadSignalHook::AspectChanged);
		Cast<AFGBuildableRailroadSignal>(sender)->mOnBlockValidationChangedDelegate.AddDynamic(this, &UFIRRailroadSignalHook::ValidationChanged);
	}

	void Unregister() override {
		Cast<AFGBuildableRailroadSignal>(Sender)->mOnAspectChangedDelegate.RemoveDynamic(this, &UFIRRailroadSignalHook::AspectChanged);
		Cast<AFGBuildableRailroadSignal>(Sender)->mOnBlockValidationChangedDelegate.RemoveDynamic(this, &UFIRRailroadSignalHook::ValidationChanged);
	}
};

UCLASS()
class UFIRPipeHyperStartHook : public UFIRMultiFunctionHook {
	GENERATED_BODY()

	protected:
	static UFIRPipeHyperStartHook* StaticSelf() {
		static UFIRPipeHyperStartHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFIRPipeHyperStartHook*>(GetDefault<UFIRPipeHyperStartHook>());
		return Hook;
	}

	// Begin UFIRFunctionHook
	virtual UFIRMultiFunctionHook* Self() {
		return StaticSelf();
	}
	// End UFIRFunctionHook

private:

	static void EnterHyperPipe(const bool& retVal, UFGCharacterMovementComponent* CharacterMovementConstants, AFGPipeHyperStart* HyperStart) {
		if(retVal && IsValid(HyperStart)) {
			StaticSelf()->Send(HyperStart, "PlayerEntered", { FIRBool(retVal)});
		}
	}
	static void ExitHyperPipe(CallScope<void(*)(UFGCharacterMovementComponent*, bool)>& call, UFGCharacterMovementComponent* charMove, bool bRagdoll){
		AActor* actor = charMove->GetTravelingPipeHyperActor();
		UObject* obj = dynamic_cast<UObject*>(actor);
		auto v = charMove->mPipeData.mConnectionToEjectThrough;
		if(IsValid(v)) {
			UFGPipeConnectionComponentBase* connection = v->mConnectedComponent;
			if(IsValid(connection)) {
				StaticSelf()->Send(connection->GetOwner(), "PlayerExited", {});
			}
		}
	}

public:
	void RegisterFuncHook() override {
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGCharacterMovementComponent::EnterPipeHyper, (void*)GetDefault<UFGCharacterMovementComponent>(), &EnterHyperPipe);
		SUBSCRIBE_METHOD_VIRTUAL(UFGCharacterMovementComponent::PipeHyperForceExit, (void*)GetDefault<UFGCharacterMovementComponent>(), &ExitHyperPipe);
    }
};

UCLASS()
class UFIRFactoryConnectorHook : public UFIRFunctionHook {
	GENERATED_BODY()

protected:
	static UFIRFactoryConnectorHook* StaticSelf() {
		static UFIRFactoryConnectorHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFIRFactoryConnectorHook*>(GetDefault<UFIRFactoryConnectorHook>());
		return Hook;
	}

	// Begin UFIRFunctionHook
	virtual UFIRFunctionHook* Self() {
		return StaticSelf();
	}
	// End UFIRFunctionHook

private:
	static FCriticalSection MutexFactoryGrab;
	static TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> FactoryGrabsRunning;

	static void LockFactoryGrab(UFGFactoryConnectionComponent* comp) {
		MutexFactoryGrab.Lock();
		++FactoryGrabsRunning.FindOrAdd(comp);
		MutexFactoryGrab.Unlock();
	}

	static bool UnlockFactoryGrab(UFGFactoryConnectionComponent* comp) {
		MutexFactoryGrab.Lock();
		int8* i = FactoryGrabsRunning.Find(comp);
		bool valid = false;
		if (i) {
			--*i;
			valid = (*i <= 0);
			if (valid) FactoryGrabsRunning.Remove(comp);
		}
		MutexFactoryGrab.Unlock();
		return valid;
	}

	static void DoFactoryGrab(UFGFactoryConnectionComponent* c, FInventoryItem& item) {
		StaticSelf()->Send(c, "ItemTransfer", {FIRAny(FInventoryItem(item))});
	}

	static void FactoryGrabHook(CallScope<bool(*)(UFGFactoryConnectionComponent*, FInventoryItem&, float&, TSubclassOf<UFGItemDescriptor>)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, float& offset, TSubclassOf<UFGItemDescriptor> type) {
		if (!StaticSelf()->IsSender(c)) return;
		LockFactoryGrab(c);
		scope(c, item, offset, type);
		if (UnlockFactoryGrab(c) && scope.GetResult()) {
			DoFactoryGrab(c, item);
		}
	}

	static void FactoryGrabInternalHook(CallScope<bool(*)(UFGFactoryConnectionComponent*, FInventoryItem&, TSubclassOf<UFGItemDescriptor>)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, TSubclassOf< UFGItemDescriptor > type) {
		if (!StaticSelf()->IsSender(c)) return;
		LockFactoryGrab(c);
		scope(c, item, type);
		if (UnlockFactoryGrab(c) && scope.GetResult()) {
			DoFactoryGrab(c, item);
		}
	}

public:
	void RegisterFuncHook() override {
		// TODO: Check if this works now
		// SUBSCRIBE_METHOD_MANUAL("?Factory_GrabOutput@UFGFactoryConnectionComponent@@QEAA_NAEAUFInventoryItem@@AEAMV?$TSubclassOf@VUFGItemDescriptor@@@@@Z", UFGFactoryConnectionComponent::Factory_GrabOutput, &FactoryGrabHook);
		SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::Factory_GrabOutput, &FactoryGrabHook);
		SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::Factory_Internal_GrabOutputInventory, &FactoryGrabInternalHook);
    }
};

UCLASS()
class UFIRPipeConnectorHook : public UFIRFunctionHook {
	GENERATED_BODY()

protected:
	static UFIRPipeConnectorHook* StaticSelf() {
		static UFIRPipeConnectorHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFIRPipeConnectorHook*>(GetDefault<UFIRPipeConnectorHook>());
		return Hook;
	}

	// Begin UFIRFunctionHook
	virtual UFIRFunctionHook* Self() {
		return StaticSelf();
	}
	// End UFIRFunctionHook

public:
	void RegisterFuncHook() override {
		// TODO: Check if this works now
		// SUBSCRIBE_METHOD_MANUAL("?Factory_GrabOutput@UFGFactoryConnectionComponent@@QEAA_NAEAUFInventoryItem@@AEAMV?$TSubclassOf@VUFGItemDescriptor@@@@@Z", UFGFactoryConnectionComponent::Factory_GrabOutput, &FactoryGrabHook);
    }
};

UCLASS()
class UFIRPowerCircuitHook : public UFIRFunctionHook {
	GENERATED_BODY()

protected:
	static UFIRPowerCircuitHook* StaticSelf() {
		static UFIRPowerCircuitHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFIRPowerCircuitHook*>(GetDefault<UFIRPowerCircuitHook>());
		return Hook;
	}

	// Begin UFIRFunctionHook
	virtual UFIRFunctionHook* Self() {
		return StaticSelf();
	}
	// End UFIRFunctionHook

private:
	static void TickCircuitHook_Decl(UFGPowerCircuit*, float);
	static void TickCircuitHook(CallScope<void(*)(UFGPowerCircuit*, float)>& scope, UFGPowerCircuit* circuit, float dt) {
		bool oldFused = circuit->IsFuseTriggered();
		scope(circuit, dt);
		bool fused = circuit->IsFuseTriggered();
		if (oldFused != fused) try {
			FScopeLock Lock(&StaticSelf()->Mutex);
			TWeakObjectPtr<UObject>* sender = StaticSelf()->Senders.Find(circuit);
			if (sender) {
				UObject* obj = sender->Get();

				StaticSelf()->Send(obj, "PowerFuseChanged", {});
			}
		} catch (...) {}
	}

public:
	void RegisterFuncHook() override {
		// TODO: Check if this works now
		//SUBSCRIBE_METHOD_MANUAL("?TickCircuit@UFGPowerCircuit@@MEAAXM@Z", TickCircuitHook_Decl, &TickCircuitHook);
		//SUBSCRIBE_METHOD(UFGPowerCircuit::TickCircuit, &TickCircuitHook);
    }
};

UCLASS()
class UFIRDimensionalDepotHook : public UFIRStaticHook {
	GENERATED_BODY()

	UPROPERTY()
	UFIRSignal* NewItemAddedSignal;

	UPROPERTY()
	UFIRSignal* ItemAmountUpdatedSignal;

	UPROPERTY()
	UFIRSignal* ItemLimitReachedUpdatedSignal;

public:
	UFUNCTION()
	void NewItem(TSubclassOf<UFGItemDescriptor> Item) {
		NewItemAddedSignal->Trigger(Sender, {(FIRClass)Item});
	}

	UFUNCTION()
	void AmountUpdated(TSubclassOf<UFGItemDescriptor> Item, int32 Amount) {
		ItemAmountUpdatedSignal->Trigger(Sender, {(FIRClass)Item, (FIRInt)Amount});
	}

	UFUNCTION()
	void LimitedReachedUpdated(TSubclassOf<UFGItemDescriptor> Item, bool Reached) {
		ItemLimitReachedUpdatedSignal->Trigger(Sender, {(FIRClass)Item, Reached});
	}

	void Register(UObject* sender) override {
		Super::Register(sender);

		UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(Sender->GetClass());
		NewItemAddedSignal = Class->FindFIRSignal(TEXT("NewItemAdded"));
		ItemAmountUpdatedSignal = Class->FindFIRSignal(TEXT("ItemAmountUpdated"));
		ItemLimitReachedUpdatedSignal = Class->FindFIRSignal(TEXT("ItemLimitReachedUpdated"));

		Cast<AFGCentralStorageSubsystem>(sender)->mOnCentralStorageNewItemAddedDelegate.AddDynamic(this, &UFIRDimensionalDepotHook::NewItem);
		Cast<AFGCentralStorageSubsystem>(sender)->mOnCentralStorageItemAmountUpdatedDelegate.AddDynamic(this, &UFIRDimensionalDepotHook::AmountUpdated);
		Cast<AFGCentralStorageSubsystem>(sender)->mOnCentralStorageItemLimitReachedUpdated.AddDynamic(this, &UFIRDimensionalDepotHook::LimitedReachedUpdated);
	}

	void Unregister() override {
		Cast<AFGCentralStorageSubsystem>(Sender)->mOnCentralStorageNewItemAddedDelegate.RemoveDynamic(this, &UFIRDimensionalDepotHook::NewItem);
		Cast<AFGCentralStorageSubsystem>(Sender)->mOnCentralStorageItemAmountUpdatedDelegate.RemoveDynamic(this, &UFIRDimensionalDepotHook::AmountUpdated);
		Cast<AFGCentralStorageSubsystem>(Sender)->mOnCentralStorageItemLimitReachedUpdated.RemoveDynamic(this, &UFIRDimensionalDepotHook::LimitedReachedUpdated);
	}
};
