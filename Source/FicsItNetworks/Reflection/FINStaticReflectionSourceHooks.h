#pragma once

#include "FGFactoryConnectionComponent.h"
#include "FGPowerCircuit.h"
#include "FGTrain.h"
#include "FINReflection.h"
#include "FINSignal.h"
#include "mod/hooking.h"
#include "Network/FINHookSubsystem.h"
#include "FINStaticReflectionSourceHooks.generated.h"

UCLASS()
class UFINStaticReflectionHook : public UFINHook {
	GENERATED_BODY()

private:
	UPROPERTY()
	UFINSignal* Signal = nullptr;

protected:
	UPROPERTY()
	UObject* Sender = nullptr;

protected:
	void Send(const TArray<FFINAnyNetworkValue>& Values) {
		Signal->Trigger(Sender, Values);
	}

public:
	void Register(UObject* sender) override {
		Super::Register(sender);

		Sender = sender;
	}
};

UCLASS()
class UFINFunctionHook : public UFINHook {
	GENERATED_BODY()

private:
	UPROPERTY()
	UObject* Sender;
	
	static TMap<UClass*, bool> IsRegistered;

protected:
	static TMap<UClass*, TSet<FWeakObjectPtr>> Senders;
	static TMap<UClass*, TSharedRef<FCriticalSection>> Mutex;

	static FCriticalSection* GetMutex(UClass* Class) {
		TSharedPtr<FCriticalSection> Section = Mutex.FindOrAdd(Class);
		return Section.Get();
	}
	
	bool IsSender(UObject* Obj) const {
		FScopeLock Lock(GetMutex(GetClass()));
		return Senders.FindOrAdd(GetClass()).Contains(Obj);
	}
	
	void Send(UObject* Obj, const FString& SignalName, const TArray<FFINAnyNetworkValue>& Data) const {
		static UFINSignal* Signal = nullptr;
		if (!Signal) {
			UFINSignal** SigPtr = FFINReflection::Get()->FindClass(Obj->GetClass())->GetSignals().FindByPredicate([&SignalName](UFINSignal* Signal) {
                return Signal->GetInternalName() == SignalName;
            });
			if (SigPtr) Signal = *SigPtr;
		}
		Signal->Trigger(Obj, Data);
	}

	virtual void RegisterFuncHook() {}

public:		
	void Register(UObject* sender) override {
		Super::Register(sender);
		
		FScopeLock Lock(GetMutex(GetClass()));
    	Senders.FindOrAdd(GetClass()).Add(Sender = sender);

		if (!IsRegistered.FindOrAdd(GetClass())) {
			IsRegistered.FindOrAdd(GetClass()) = true;
			RegisterFuncHook();
		}
    }
		
	void Unregister() override {
		FScopeLock Lock(GetMutex(GetClass()));
    	Senders.FindOrAdd(GetClass()).Remove(Sender);
    }
};

UCLASS()
class UFINTrainHook : public UFINStaticReflectionHook {
	GENERATED_BODY()
			
public:	
	UFUNCTION()
	void SelfDrvingUpdate(bool enabled) {
		Send({enabled});
	}
			
	void Register(UObject* sender) override {
		Super::Register(sender);
		
		Cast<AFGTrain>(sender)->mOnSelfDrivingChanged.AddDynamic(this, &UFINTrainHook::SelfDrvingUpdate);
	}
		
	void Unregister() override {
		Cast<AFGTrain>(Sender)->mOnSelfDrivingChanged.RemoveDynamic(this, &UFINTrainHook::SelfDrvingUpdate);
	}
};

UCLASS()
class UFINFactoryConnectorHook : public UFINFunctionHook {
	GENERATED_BODY()
			
private:
	static FCriticalSection MutexFactoryGrab;
	static TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> FactoryGrabsRunning;

	static const UFINFactoryConnectorHook* Self() {
		static const UFINFactoryConnectorHook* Hook = nullptr;
		if (!Hook) Hook = GetDefault<UFINFactoryConnectorHook>();
		return Hook;
	}

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
		Self()->Send(c, "ItemTransfer", {FINAny(FInventoryItem(item))});
	}

	static void FactoryGrabHook(CallScope<bool(*)(UFGFactoryConnectionComponent*, FInventoryItem&, float&, TSubclassOf<UFGItemDescriptor>)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, float& offset, TSubclassOf<UFGItemDescriptor> type) {
		if (!Self()->IsSender(c)) return;
		LockFactoryGrab(c);
		scope(c, item, offset, type);
		if (UnlockFactoryGrab(c) && scope.getResult()) {
			DoFactoryGrab(c, item);
		}
	}

	static void FactoryGrabInternalHook(CallScope<bool(*)(UFGFactoryConnectionComponent*, FInventoryItem&, TSubclassOf<UFGItemDescriptor>)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, TSubclassOf< UFGItemDescriptor > type) {
		if (!Self()->IsSender(c)) return;
		LockFactoryGrab(c);
		scope(c, item, type);
		if (UnlockFactoryGrab(c) && scope.getResult()) {
			DoFactoryGrab(c, item);
		}
	}
			
public:		
	void RegisterFuncHook() override {
		SUBSCRIBE_METHOD_MANUAL("?Factory_GrabOutput@UFGFactoryConnectionComponent@@QEAA_NAEAUFInventoryItem@@AEAMV?$TSubclassOf@VUFGItemDescriptor@@@@@Z", UFGFactoryConnectionComponent::Factory_GrabOutput, &FactoryGrabHook);
		SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::Factory_Internal_GrabOutputInventory, &FactoryGrabInternalHook);
    }
};

UCLASS()
class UFINPowerCircuitHook : public UFINFunctionHook {
	GENERATED_BODY()
			
private:
	static void TickCircuitHook_Decl(UFGPowerCircuit*, float);
	static void TickCircuitHook(CallScope<void(*)(UFGPowerCircuit*, float)>& scope, UFGPowerCircuit* circuit, float dt) {
		bool oldFused = circuit->IsFuseTriggered();
		scope(circuit, dt);
		bool fused = circuit->IsFuseTriggered();
		if (oldFused != fused) try {
			GetMutex(StaticClass())->Lock();
			FWeakObjectPtr* sender = Senders.FindOrAdd(StaticClass()).Find(circuit);
			if (sender) {
				UObject* obj = sender->Get();

				GetDefault<UFINPowerCircuitHook>()->Send(obj, "PowerFuseChanged", {});
			}
			GetMutex(StaticClass())->Unlock();
		} catch (...) {}
	}
			
public:
	void RegisterFuncHook() override {
		SUBSCRIBE_METHOD_MANUAL("?TickCircuit@UFGPowerCircuit@@MEAAXM@Z", TickCircuitHook_Decl, &TickCircuitHook);
    }
};
