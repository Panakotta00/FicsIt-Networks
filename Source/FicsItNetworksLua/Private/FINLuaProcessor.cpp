#include "FINLuaProcessor.h"

#include "Async/AsyncWork.h"
#include "Misc/Base64.h"
#include "Buildables/FGBuildable.h"
#include "FGInventoryComponent.h"
#include "FGPlayerState.h"
#include "FicsItLogLibrary.h"
#include "FicsItNetworksLuaModule.h"
#include "FILLogContainer.h"
#include "FINComputerEEPROMDesc.h"
#include "FicsItKernel/Processor/FINItemStateEEPROMText.h"
#include "FINMediaSubsystem.h"
#include "Signals/FINSignalSubsystem.h"
#include "FINLua/LuaEventAPI.h"
#include "FINLua/LuaFuture.h"
#include "FINLua/API/LuaKernelAPI.h"
#include "FINLua/API/LuaWorldAPI.h"
#include "OnlineSubsystemModule.h"
#include "Engine/Engine.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/LuaUtil.h"
#include "GameFramework/PlayerState.h"
#include "Signals/FINSignalData.h"

void LuaFileSystemListener::onUnmounted(CodersFileSystem::Path path, TSharedRef<CodersFileSystem::Device> device) {
	/*for (FINLua::LuaFile file : Parent->GetFileStreams()) {
		if (!Parent->GetKernel()->GetFileSystem()) {
			file->file->close();
		}
	}*/
}

void LuaFileSystemListener::onNodeRemoved(CodersFileSystem::Path path, CodersFileSystem::NodeType type) {
	/*for (FINLua::LuaFile file : Parent->GetFileStreams()) {
		if (file->path.length() > 0 && (!Parent->GetKernel()->GetFileSystem())) {
			file->file->close();
		}
	}*/
}

UFINLuaProcessor::UFINLuaProcessor() {
	Runtime.Runtime.Modules.Add("DebugModule");
	Runtime.Runtime.Modules.Add("LogModule");
	Runtime.Runtime.Modules.Add("KernelModule");
	Runtime.Runtime.Modules.Add("ComputerModule");
	Runtime.Runtime.Modules.Add("ComponentModule");
	Runtime.Runtime.Modules.Add("WorldModule");
	Runtime.Runtime.Modules.Add("EventModule");
	Runtime.Runtime.Modules.Add("FutureModule");
	Runtime.Runtime.Modules.Add("FileSystemModule");
	Runtime.Runtime.OnPreLuaTick.AddWeakLambda(this, [this](TArray<TSharedPtr<void>>& TickStack) {
		TickStack.Add(MakeShared<FFILLogScope>(GetKernel()->GetLog()));
	});
	Runtime.Runtime.OnPreModules.AddWeakLambda(this, [this]() {
		FINLua::luaFIN_setReferenceCollector(Runtime.Runtime.GetLuaState(), ReferenceCollector);
		FINLua::luaFIN_setWorld(Runtime.Runtime.GetLuaState(), GetWorld());
		FINLua::luaFIN_setComponentNetwork(Runtime.Runtime.GetLuaState(), &ComponentNetwork);
		FINLua::luaFIN_setKernel(Runtime.Runtime.GetLuaState(), GetKernel());
		FINLua::luaFIN_setFileSystem(Runtime.Runtime.GetLuaState(), GetKernel()->GetFileSystem());
		FINLua::luaFIN_setEventSystem(Runtime.Runtime.GetLuaState(), EventSystem);
		FINLua::luaFIN_createFutureDelegate(Runtime.Runtime.GetLuaState()).AddWeakLambda(this, [this](const FINLua::FLuaFuture& Future) {
			GetKernel()->PushFuture(Future);
		});
	});
	Runtime.Runtime.OnPostReset.AddWeakLambda(this, [this]() {
		TOptional<FString> error = Runtime.Runtime.LoadState(RuntimeState);
		if (error) {
			GetKernel()->Reset();
			FString message = FString::Printf(TEXT("%s: Unable to load computer state from save-file (computer will restart): %s"), *DebugInfo, **error);
			UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s"), *message);
			GetKernel()->GetLog()->PushLogEntry(FIL_Verbosity_Warning, message);
		}
	});

	ComponentNetwork.OnGetComponentByID.BindWeakLambda(this, [this](const FGuid& ID) {
		return GetKernel()->GetNetwork()->GetComponentByID(ID);
	});
	ComponentNetwork.OnGetComponentByNick.BindWeakLambda(this, [this](const FString& Nick) {
		return GetKernel()->GetNetwork()->GetComponentByNick(Nick);
	});
	ComponentNetwork.OnGetComponentByClass.BindWeakLambda(this, [this](UClass* Class, bool bInRedirect) {
		return GetKernel()->GetNetwork()->GetComponentByClass(Class, bInRedirect);
	});

	EventSystem.OnTimeSinceStart.BindWeakLambda(this, [this]() {
		return GetKernel()->GetTimeSinceStart();
	});
	EventSystem.OnListen.BindWeakLambda(this, [this](FFIRTrace Object) {
		UFINKernelSystem* kernel = GetKernel();
		UFINKernelNetworkController* network = kernel->GetNetwork();
		AFINSignalSubsystem::GetSignalSubsystem(GetKernel())->Listen(Object.GetUnderlyingPtr(), Object.Reverse() / network->GetComponent().GetObject());
	});
	EventSystem.OnListening.BindWeakLambda(this, [this]() {
		UObject* comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
		return AFINSignalSubsystem::GetSignalSubsystem(comp)->GetListening(comp);
	});
	EventSystem.OnIgnoreAll.BindWeakLambda(this, [this]() {
		UObject* comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
		AFINSignalSubsystem::GetSignalSubsystem(comp)->IgnoreAll(comp);
	});
	EventSystem.OnClear.BindWeakLambda(this, [this]() {
		GetKernel()->GetNetwork()->ClearSignals();
	});
	EventSystem.OnIgnore.BindWeakLambda(this, [this](UObject* Object) {
		UObject* comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
		AFINSignalSubsystem::GetSignalSubsystem(comp)->Ignore(comp, Object);
	});
	EventSystem.OnPullSignal.BindWeakLambda(this, [this]() -> TOptional<TTuple<FFIRTrace, FFINSignalData>> {
		if (GetKernel()->GetNetwork()->GetSignalCount() < 1) {
			return {};
		}
		FFIRTrace sender;
		FFINSignalData data = GetKernel()->GetNetwork()->PopSignal(sender);
		return {{sender, data}};
	});
}

void UFINLuaProcessor::BeginDestroy() {
	Super::BeginDestroy();
	Runtime.Runtime.Destroy();
}

void UFINLuaProcessor::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(Kernel);
	out_dependentObjects.Add(AFINMediaSubsystem::GetMediaSubsystem(this));
}

void UFINLuaProcessor::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	Runtime.PauseAndWait();
	RuntimeState.Clear();
	if (GetKernel()->GetState() != FIN_KERNEL_RUNNING) return;
	RuntimeState = Runtime.Runtime.SaveState();
	if (RuntimeState.IsFailure()) {
		FString message = FString::Printf(TEXT("%s: Unable to save computer state into a save-file (computer will restart when loading the save-file): %s"), *DebugInfo, *RuntimeState.Failure);
		UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s"), *message);
		GetKernel()->GetLog()->PushLogEntry(FIL_Verbosity_Warning, message);
	}

	bIsFromSave = true;
}

void UFINLuaProcessor::PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {}

void UFINLuaProcessor::PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {}

void UFINLuaProcessor::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	if (bIsFromSave) {
		Runtime.Runtime.Reset();
	}
}

void UFINLuaProcessor::SetKernel(UFINKernelSystem* InKernel) {
	//if (GetKernel() && GetKernel()->GetFileSystem()) GetKernel()->GetFileSystem()->removeListener(FileSystemListener);
	Kernel = InKernel;
}

UE_DISABLE_OPTIMIZATION_SHIP
void UFINLuaProcessor::Tick(float InDelta) {
	if (GetKernel()->GetNetwork()->GetSignalCount() > 0) {
		Runtime.Runtime.Timeout.Reset();
	}

	Runtime.Runtime.Hook_Tick = 2500;
	Runtime.Run();

	switch (Runtime.GetStatus()) {
	case FFINLuaRuntime::Finished:
		GetKernel()->Stop();
		break;
	case FFINLuaRuntime::Crashed:
		GetKernel()->Crash(MakeShared<FFINKernelCrash>(*Runtime.Runtime.GetError()));
		break;
	default: break;
	}

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	AFGPlayerState* playerState = Cast<AFGPlayerState>(playerController->PlayerState);
	if (playerState) {
		auto& identity = playerState->GetClientIdentity();
		auto& onlineIdRegistryRegistry = UE::Online::FOnlineIdRegistryRegistry::Get();
		for (const auto& [service, accountIdRef] : identity.AccountIds) {
			FString accountId = onlineIdRegistryRegistry.ToString(accountIdRef);
			if (service == UE::Online::EOnlineServices::Steam) {
				uint64 num = FCString::Strtoi64(*accountId, nullptr, 16);
				num = ByteSwap(num);
				accountId = FString::Printf(TEXT("%llu"), num);
			}
			UE_LOG(LogTemp, Warning, TEXT("Account Id for %s: %s"), LexToString(service), *accountId);
		}
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Player State"));
	}
}
UE_ENABLE_OPTIMIZATION_SHIP

void UFINLuaProcessor::Stop(bool bIsCrash) {
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor stop %s"), *DebugInfo, bIsCrash ? TEXT("due to crash") : TEXT(""));
	Runtime.Runtime.Destroy();
}

void UFINLuaProcessor::Reset() {
	UObject* comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
	AFINSignalSubsystem::GetSignalSubsystem(comp)->IgnoreAll(comp);
	Kernel->GetNetwork()->ClearSignals();

	RuntimeState.Clear();

	Runtime.Runtime.Reset();

	TOptional<FString> Code = GetEEPROM();
	if (Code) {
		TOptional<FString> error = Runtime.Runtime.LoadCode(*Code);
		if (error) {
			GetKernel()->Crash(MakeShared<FFINKernelCrash>(*error));
		}
	}
}

TOptional<FString> UFINLuaProcessor::GetEEPROM() const {
	FInventoryItem eeprom = Kernel->GetEEPROM();
	if (const FFINItemStateEEPROMText* state = eeprom.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		return state->Code;
	}
	return {};
}

bool UFINLuaProcessor::SetEEPROM(const FString& Code) {
	FInventoryItem eeprom = Kernel->GetEEPROM();
	UFINComputerEEPROMDesc::CreateEEPROMStateInItem(eeprom);

	if (const FFINItemStateEEPROMText* stateLua = eeprom.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		FFINItemStateEEPROMText state = *stateLua;
		state.Code = Code;
		return Kernel->SetEEPROM(FFGDynamicStruct(state));
	}

	return false;
}
