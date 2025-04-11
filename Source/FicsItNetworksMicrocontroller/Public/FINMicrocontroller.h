#pragma once

#include "CoreMinimal.h"
#include "FINLuaReferenceCollector.h"
#include "FINLuaRuntime.h"
#include "FINLuaRuntimePersistence.h"
#include "FINNetworkCircuitNode.h"
#include "FINNetworkMessageInterface.h"
#include "FINSignalListener.h"
#include "FIRSourceUObject.h"
#include "LuaComponentAPI.h"
#include "LuaEventAPI.h"
#include "Buildables/FGBuildable.h"
#include "NetworkController.h"
#include "FINMicrocontroller.generated.h"

class UFINMicrocontrollerReference;

UENUM(BlueprintType)
enum EFINMicrocontrollerState {
	FIN_Microcontroller_State_Stopped,
	FIN_Microcontroller_State_Running,
	FIN_Microcontroller_State_Failed,
};

UCLASS(Blueprintable)
class FICSITNETWORKSMICROCONTROLLER_API AFINMicrocontroller : public AFGBuildable, public IFINSignalListener, public IFINNetworkCircuitNode, public IFINNetworkComponent, public IFINNetworkMessageInterface {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly)
	UFGInventoryComponent* Inventory;

	UPROPERTY(SaveGame, BlueprintReadOnly)
	UFILLogContainer* Log;

	UPROPERTY(SaveGame)
	UFINKernelNetworkController* NetworkController;

	UPROPERTY(SaveGame)
	AActor* NetworkComponent = nullptr;

	UPROPERTY(SaveGame)
	FGuid ID;

	UPROPERTY(SaveGame)
	FString Nick;

	UPROPERTY(SaveGame)
	TSet<int> OpenPorts;

	UPROPERTY()
	UFINMicrocontrollerReference* Reference = nullptr;

	UPROPERTY()
	AFINNetworkCircuit* NetworkCircuit = nullptr;

	const int ProxyLimit = 5;

	AFINMicrocontroller();
	~AFINMicrocontroller();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	// Begin IFINSignalListener
	virtual void HandleSignal(const FFINSignalData& Signal, const FFIRTrace& Sender) override;
	// End IFINSignalListener

	// Begin IFINNetworkCircuitNode
	TSet<UObject*> GetConnected_Implementation() const;
	AFINNetworkCircuit* GetCircuit_Implementation() const;
	void SetCircuit_Implementation(AFINNetworkCircuit* Circuit);
	// End IFINNetworkCircuitNode

	// Begin IFINNetworkComponent
	FGuid GetID_Implementation() const { return ID; }
	FString GetNick_Implementation() const { return Nick; }
	void SetNick_Implementation(const FString& nick) { Nick = nick; }
	bool HasNick_Implementation(const FString& nick) { return HasNickByNick(Nick, nick); }
	UObject* GetInstanceRedirect_Implementation() { return this; }
	bool AccessPermitted_Implementation(FGuid InID) const { return true; }
	// End IFINNetworkComponent

	// Begin IFINNetworkMessageInterface
	virtual bool IsPortOpen(int Port) { return OpenPorts.Contains(Port); }
	virtual void HandleMessage(const FGuid& ID, const FGuid& Sender, const FGuid& Receiver, int Port, const FIRArray& Data);
	virtual bool IsNetworkMessageRouter() const { return false; }
	// End IFINNetworkMessageInterface

	UFUNCTION(BlueprintCallable)
	void ToggleRuntime();
	UFUNCTION(BlueprintCallable)
	void StartRuntime();
	UFUNCTION(BlueprintCallable)
	void StopRuntime();
	UFUNCTION(BlueprintCallable)
	void CrashRuntime(const FString& message);

	TOptional<FString> GetCode() const;
	UFUNCTION(BlueprintCallable)
	FString GetCode(const FString& Default) const;
	UFUNCTION(BlueprintCallable)
	void SetCode(const FString& Code);

	UFUNCTION(BlueprintCallable)
	FString GetDebugInfo() const;
	UFUNCTION(BlueprintCallable)
	UFILLogContainer* GetLog() const;

	UFUNCTION(BlueprintCallable)
	EFINMicrocontrollerState GetStatus() const;

	UFUNCTION()
	void netClass_Meta(FString& InternalName, FText& DisplayName, FFIRFunctionMeta& netFuncMeta_open) {
		InternalName = TEXT("Microcontroller");
		DisplayName = FText::FromString(TEXT("Microcontroller"));
	}
	UFUNCTION()
	void netSig_NetworkMessage(const FString& sender, int port, TArray<FFIRAnyValue> varargs) {}

protected:
	void SetupRuntime();

	UPROPERTY(SaveGame)
	TSet<FGuid> Proxies;

	FFINLuaRuntime Runtime;
	UPROPERTY()
	FFINLuaReferenceCollector ReferenceCollector;
	FFINLuaComponentNetworkAccessDelegates ComponentNetwork;
	FFINLuaEventSystem EventSystem;
	UPROPERTY(SaveGame)
	FString Error;

	UPROPERTY(SaveGame)
	FFINLuaRuntimePersistenceState PersistenceState;

	UPROPERTY()
	TSet<FGuid> HandledMessages;
	FCriticalSection HandledMessagesMutex;
};
