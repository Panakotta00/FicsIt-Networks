#pragma once

#include "FGBuildableManufacturer.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPowerCircuit.h"
#include "FGTrain.h"
#include "LuaInstance.h"
#include "Network/FINHookSubsystem.h"
#include "Delegates/DelegateSignatureImpl.inl"
#include "mod/hooking.h"
#include "Network/FINFuture.h"
#include "Network/Signals/FINSmartSignal.h"


#include "LuaLib.generated.h"

UCLASS()
class UFINTrainHook : public UFINHook {
	GENERATED_BODY()
			
private:
	UPROPERTY()
	UObject* Sender = nullptr;
			
public:	
	UFUNCTION()
	void SelfDrvingUpdate(bool enabled) {
		AFINHookSubsystem::GetHookSubsystem(this)->EmitSignal(Sender, FFINSmartSignal("SelfDrvingUpdate", enabled));
	}
			
	void Register(UObject* sender) override {
		Sender = sender;
		Cast<AFGTrain>(sender)->mOnSelfDrivingChanged.AddDynamic(this, &UFINTrainHook::SelfDrvingUpdate);
	}
		
	void Unregister() override {
		Cast<AFGTrain>(Sender)->mOnSelfDrivingChanged.RemoveDynamic(this, &UFINTrainHook::SelfDrvingUpdate);
	}
};

UCLASS()
class UFINFactoryConnectorHook : public UFINHook {
	GENERATED_BODY()
			
private:
	UPROPERTY()
	UObject* Sender;
	
    static TSet<FWeakObjectPtr> Senders;
	static bool registered;

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
		AFINHookSubsystem::GetHookSubsystem(c)->EmitSignal(c, FFINSmartSignal("ItemTransfer", TFINDynamicStruct<FInventoryItem>(item)));
	}

	static void FactoryGrabHook(CallScope<bool(*)(UFGFactoryConnectionComponent*, FInventoryItem&, float&, TSubclassOf<UFGItemDescriptor>)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, float& offset, TSubclassOf<UFGItemDescriptor> type) {
		if (!Senders.Contains(c)) return;
		LockFactoryGrab(c);
		scope(c, item, offset, type);
		if (UnlockFactoryGrab(c) && scope.getResult()) {
			DoFactoryGrab(c, item);
		}
	}

	static void FactoryGrabInternalHook(CallScope<bool(*)(UFGFactoryConnectionComponent*, FInventoryItem&, TSubclassOf<UFGItemDescriptor>)>& scope, UFGFactoryConnectionComponent* c, FInventoryItem& item, TSubclassOf< UFGItemDescriptor > type) {
		if (!Senders.Contains(c)) return;
		LockFactoryGrab(c);
		scope(c, item, type);
		if (UnlockFactoryGrab(c) && scope.getResult()) {
			DoFactoryGrab(c, item);
		}
	}
			
public:		
	void Register(UObject* sender) override {
    	Senders.Add(Sender = sender);

		if (!registered) {
			registered = true;

			SUBSCRIBE_METHOD_MANUAL("?Factory_GrabOutput@UFGFactoryConnectionComponent@@QEAA_NAEAUFInventoryItem@@AEAMV?$TSubclassOf@VUFGItemDescriptor@@@@@Z", UFGFactoryConnectionComponent::Factory_GrabOutput, &FactoryGrabHook);
			SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::Factory_Internal_GrabOutputInventory, &FactoryGrabInternalHook);
		}
    }
		
	void Unregister() override {
    	Senders.Remove(Sender);
    }
};

UCLASS()
class UFINPowerCircuitHook : public UFINHook {
	GENERATED_BODY()
			
private:
	UPROPERTY()
	UObject* Sender;
	
    static TSet<FWeakObjectPtr> Senders;
	static bool registered;

	static FCriticalSection Mutex;
	static TMap<TWeakObjectPtr<UFGFactoryConnectionComponent>, int8> FactoryGrabsRunning;

	static void TickCircuitHook_Decl(UFGPowerCircuit*, float);
	static void TickCircuitHook(CallScope<void(*)(UFGPowerCircuit*, float)>& scope, UFGPowerCircuit* circuit, float dt) {
		bool oldFused = circuit->IsFuseTriggered();
		scope(circuit, dt);
		bool fused = circuit->IsFuseTriggered();
		if (oldFused != fused) try {
			Mutex.Lock();
			FWeakObjectPtr* sender = Senders.Find(circuit);
			if (sender) {
				UObject* obj = sender->Get();
				AFINHookSubsystem::GetHookSubsystem(obj)->EmitSignal(obj, FFINSmartSignal("PowerFuseChanged"));
			}
			Mutex.Unlock();
		} catch (...) {}
	}
			
public:		
	void Register(UObject* sender) override {
		Mutex.Lock();
    	Senders.Add(Sender = sender);

		if (!registered) {
			registered = true;
			
			SUBSCRIBE_METHOD_MANUAL("?TickCircuit@UFGPowerCircuit@@MEAAXM@Z", TickCircuitHook_Decl, &TickCircuitHook);
		}
		Mutex.Unlock();
    }
		
	void Unregister() override {
		Mutex.Lock();
    	Senders.Remove(Sender);
		Mutex.Unlock();
    }
};

USTRUCT()
struct FFINManufacturerSetRecipeFuture : public FFINFutureSimpleDone {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TWeakObjectPtr<AFGBuildableManufacturer> Manufacturer;

	UPROPERTY(SaveGame)
	TSubclassOf<UFGRecipe> Recipe;

	UPROPERTY(SaveGame)
	bool bGotSet = false;
	
	FFINManufacturerSetRecipeFuture() = default;
	FFINManufacturerSetRecipeFuture(TWeakObjectPtr<AFGBuildableManufacturer> Manu, TSubclassOf<UFGRecipe> Recipe) : Manufacturer(Manu), Recipe(Recipe) {}

	virtual void Execute() override;
	virtual int operator>>(FFINValueReader& Reader) const override {
		Reader << bGotSet;
		return 1;
	}
};
