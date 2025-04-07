#include "FINLuaProcessor.h"

#include "AsyncWork.h"
#include "Base64.h"
#include "FGInventoryComponent.h"
#include "FicsItLogLibrary.h"
#include "FicsItNetworksLuaModule.h"
#include "FILLogContainer.h"
#include "FINComputerEEPROMDesc.h"
#include "FINItemStateEEPROMText.h"
#include "FINMediaSubsystem.h"
#include "FINSignalSubsystem.h"
#include "LuaEventAPI.h"
#include "LuaKernelAPI.h"
#include "LuaWorldAPI.h"
#include "Engine/Engine.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/LuaUtil.h"
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
}

void UFINLuaProcessor::PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {}

void UFINLuaProcessor::PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {}

void UFINLuaProcessor::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	Runtime.Runtime.Reset();
}

void UFINLuaProcessor::SetKernel(UFINKernelSystem* InKernel) {
	//if (GetKernel() && GetKernel()->GetFileSystem()) GetKernel()->GetFileSystem()->removeListener(FileSystemListener);
	Kernel = InKernel;
}
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
}

void UFINLuaProcessor::Stop(bool bIsCrash) {
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor stop %s"), *DebugInfo, bIsCrash ? TEXT("due to crash") : TEXT(""));
	Runtime.Runtime.Destroy();
}

void UFINLuaProcessor::Reset() {
	RuntimeState.Clear();

	Runtime.Runtime.Reset();

	TOptional<FString> Code = GetEEPROM();
	if (Code) {
		Runtime.Runtime.LoadCode(*Code);
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
