#pragma once

#include "FicsItNetworksModule.h"
#include "Network/FINHookSubsystem.h"
#include "FINReflection.h"
#include "FINSignal.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionComponent.h"
#include "FGPowerCircuit.h"
#include "FGTrain.h"
#include "Buildables/FGBuildablePipeHyper.h"
#include "Buildables/FGBuildableRailroadSignal.h"
#include "Buildables/FGPipeHyperStart.h"
#include "Patching/NativeHookManager.h"
#include "FGCharacterMovementComponent.h"
#include "FGCharacterPlayer.h"
#include "FINStaticReflectionSourceHooks.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINStaticReflectionHook : public UFINHook {
	GENERATED_BODY()

protected:
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
class FICSITNETWORKS_API UFINFunctionHook : public UFINHook {
	GENERATED_BODY()

private:
	UPROPERTY()
	UObject* Sender;
	
	bool bIsRegistered;

	UPROPERTY()
	UFINSignal* Signal = nullptr;

protected:
	UPROPERTY()
	TSet<TWeakObjectPtr<UObject>> Senders;
	FCriticalSection Mutex;
	
	bool IsSender(UObject* Obj) {
		FScopeLock Lock(&Mutex);
		return Senders.Contains(Obj);
	}
	
	void Send(UObject* Obj, const FString& SignalName, const TArray<FFINAnyNetworkValue>& Data) {
		if (!Signal) {
			UFINClass* Class = FFINReflection::Get()->FindClass(Obj->GetClass());
			Signal = Class->FindFINSignal(SignalName);
			if (!Signal) UE_LOG(LogFicsItNetworks, Error, TEXT("Signal with name '%s' not found for object '%s' of FINClass '%s'"), *SignalName, *Obj->GetName(), *Class->GetInternalName());
		}
		if (Signal) Signal->Trigger(Obj, Data);
	}

	virtual void RegisterFuncHook() {}

	virtual UFINFunctionHook* Self() { return nullptr; }

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
class UFINRailroadSignalHook : public UFINStaticReflectionHook {
	GENERATED_BODY()
			
public:
	UFUNCTION()
	void AspectChanged(ERailroadSignalAspect Aspect) {
		Send({(int64)Aspect});
	}
	
	void Register(UObject* sender) override {
		Super::Register(sender);
		
		UFINClass* Class = FFINReflection::Get()->FindClass(Sender->GetClass());
		Signal = Class->FindFINSignal(TEXT("AspectChanged"));
		
		Cast<AFGBuildableRailroadSignal>(sender)->mOnAspectChangedDelegate.AddDynamic(this, &UFINRailroadSignalHook::AspectChanged);
	}
		
	void Unregister() override {
		Cast<AFGBuildableRailroadSignal>(Sender)->mOnAspectChangedDelegate.RemoveDynamic(this, &UFINRailroadSignalHook::AspectChanged);
	}
};

UCLASS()
class UFINPipeHyperStartHook : public UFINFunctionHook {
	GENERATED_BODY()

	protected:
	static UFINPipeHyperStartHook* StaticSelf() {
		static UFINPipeHyperStartHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFINPipeHyperStartHook*>(GetDefault<UFINPipeHyperStartHook>());
		return Hook; 
	}
	
	// Begin UFINFunctionHook
	virtual UFINFunctionHook* Self() {
		return StaticSelf();
	}
	// End UFINFunctionHook

private:

	static void EnterHyperPipe(const bool& retVal, UFGCharacterMovementComponent* CharacterMovementConstants, AFGPipeHyperStart* HyperStart) {
		if(retVal && IsValid(HyperStart)) {
			StaticSelf()->Send(HyperStart, "Transport", { FINStr("Entered"), FINBool(retVal)});
		}
	}
	static void ExitHyperPipe(CallScope<void(*)(UFGCharacterMovementComponent*, bool)>& call, UFGCharacterMovementComponent* charMove, bool bRagdoll){
		AActor* actor = charMove->GetTravelingPipeHyperActor();
		UObject* obj = dynamic_cast<UObject*>(actor);
		auto v = charMove->mPipeData.mConnectionToEjectThrough;
		if(IsValid(v)) {
			UFGPipeConnectionComponentBase* connection = v->mConnectedComponent;
			if(IsValid(connection)) {
				StaticSelf()->Send(connection->GetOwner(), "Transport", {FINStr("Exited"), true, FINTrace(connection)});
			}
		}
	} 
				
public:
	void RegisterFuncHook() override {
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGCharacterMovementComponent::EnterPipeHyper, (void*)GetDefault<UFGCharacterMovementComponent>(), &EnterHyperPipe)
		SUBSCRIBE_METHOD_VIRTUAL(UFGCharacterMovementComponent::PipeHyperForceExit, (void*)GetDefault<UFGCharacterMovementComponent>(), &ExitHyperPipe)  
    }
};

UCLASS()
class UFINFactoryConnectorHook : public UFINFunctionHook {
	GENERATED_BODY()

protected:
	static UFINFactoryConnectorHook* StaticSelf() {
		static UFINFactoryConnectorHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFINFactoryConnectorHook*>(GetDefault<UFINFactoryConnectorHook>());
		return Hook; 
	}

	// Begin UFINFunctionHook
	virtual UFINFunctionHook* Self() {
		return StaticSelf();
	}
	// End UFINFunctionHook

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
		StaticSelf()->Send(c, "ItemTransfer", {FINAny(FInventoryItem(item))});
	}

	static void FactoryGrabHook(CallScope<bool(*)(UFGFactoryConnectionComponent*, FInventoryItem&, float&, TSubclassOf<UFGItemDescriptor>)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, float& offset, TSubclassOf<UFGItemDescriptor> type) {
		if (!StaticSelf()->IsSender(c)) return;
		LockFactoryGrab(c);
		scope(c, item, offset, type);
		if (UnlockFactoryGrab(c) && scope.getResult()) {
			DoFactoryGrab(c, item);
		}
	}

	static void FactoryGrabInternalHook(CallScope<bool(*)(UFGFactoryConnectionComponent*, FInventoryItem&, TSubclassOf<UFGItemDescriptor>)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, TSubclassOf< UFGItemDescriptor > type) {
		if (!StaticSelf()->IsSender(c)) return;
		LockFactoryGrab(c);
		scope(c, item, type);
		if (UnlockFactoryGrab(c) && scope.getResult()) {
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
class UFINPipeConnectorHook : public UFINFunctionHook {
	GENERATED_BODY()

protected:
	static UFINPipeConnectorHook* StaticSelf() {
		static UFINPipeConnectorHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFINPipeConnectorHook*>(GetDefault<UFINPipeConnectorHook>());
		return Hook; 
	}

	// Begin UFINFunctionHook
	virtual UFINFunctionHook* Self() {
		return StaticSelf();
	}
	// End UFINFunctionHook
			
public:		
	void RegisterFuncHook() override {
		// TODO: Check if this works now
		// SUBSCRIBE_METHOD_MANUAL("?Factory_GrabOutput@UFGFactoryConnectionComponent@@QEAA_NAEAUFInventoryItem@@AEAMV?$TSubclassOf@VUFGItemDescriptor@@@@@Z", UFGFactoryConnectionComponent::Factory_GrabOutput, &FactoryGrabHook);
    }
};

UCLASS()
class UFINPowerCircuitHook : public UFINFunctionHook {
	GENERATED_BODY()

protected:
	static UFINPowerCircuitHook* StaticSelf() {
		static UFINPowerCircuitHook* Hook = nullptr;
		if (!Hook) Hook = const_cast<UFINPowerCircuitHook*>(GetDefault<UFINPowerCircuitHook>());
		return Hook; 
	}

	// Begin UFINFunctionHook
	virtual UFINFunctionHook* Self() {
		return StaticSelf();
	}
	// End UFINFunctionHook
			
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
