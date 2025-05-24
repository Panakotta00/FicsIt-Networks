#include "Kernel/FIVSProcessor.h"

#include "FGInventoryComponent.h"
#include "FicsItReflection.h"
#include "FILLogContainer.h"
#include "FINItemStateEEPROMText.h"
#include "FINMediaSubsystem.h"
#include "FINNetworkCircuit.h"
#include "FINNetworkCircuitNode.h"
#include "FINSignalSubsystem.h"
#include "LuaFileSystemAPI.h"
#include "LuaFuture.h"
#include "LuaKernelAPI.h"
#include "LuaWorldAPI.h"
#include "NetworkController.h"
#include "Script/FIVSGraph.h"
#include "Script/Library/FIVSNode_OnTick.h"

UFIVSProcessor::UFIVSProcessor() {
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
			UE_LOG(LogFicsItVisualScript, Display, TEXT("%s"), *message);
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

void UFIVSProcessor::BeginDestroy() {
	Super::BeginDestroy();
	Runtime.Runtime.Destroy();
}

void UFIVSProcessor::Tick(float InDeltaTime) {
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

void UFIVSProcessor::Reset() {
	UObject* comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
	AFINSignalSubsystem::GetSignalSubsystem(comp)->IgnoreAll(comp);
	Kernel->GetNetwork()->ClearSignals();

	RuntimeState.Clear();

	Runtime.Runtime.Reset();

	auto state = GetKernel()->GetEEPROM().GetItemState();
	const FFINItemStateEEPROMText* eeprom = state.GetValuePtr<FFINItemStateEEPROMText>();

	if (!eeprom) {
		GetKernel()->Crash(MakeShared<FFINKernelCrash>(TEXT("No EEPROM Found")));
		return;
	}

	uint32 hash = GetTypeHash(eeprom->Code);
	if (hash != GraphHash) {
		UFIVSGraph* graph = NewObject<UFIVSGraph>();
		UFIVSSerailizationUtils::FIVS_DeserializeGraph(graph, eeprom->Code, false);

		FFIVSLuaCompilerContext luaContext;
		for (UFIVSNode* node : graph->GetNodes()) {
			if (!node->Implements<UFIVSCompileLuaInterface>()) continue;
			if (!Cast<IFIVSCompileLuaInterface>(node)->IsLuaRootNode()) continue;
			Cast<IFIVSCompileLuaInterface>(node)->CompileNodeToLua(luaContext);
		}

		LuaCode = luaContext.FinalizeCode();
		UE_LOG(LogFicsItVisualScript, Warning, TEXT("Compiled Lua Code:\n%s"), *FString(luaContext.FinalizeCode()))

		GraphHash = hash;
	}

	TOptional<FString> error = Runtime.Runtime.LoadCode(*LuaCode);
	if (error) {
		GetKernel()->Crash(MakeShared<FFINKernelCrash>(*error));
	}
}

void UFIVSProcessor::Stop(bool bIsCrash) {
	UE_LOG(LogFicsItVisualScript, Display, TEXT("%s: FIVS Processor stop %s"), *DebugInfo, bIsCrash ? TEXT("due to crash") : TEXT(""));
	Runtime.Runtime.Destroy();
}

void UFIVSProcessor::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(Kernel);
	out_dependentObjects.Add(AFINMediaSubsystem::GetMediaSubsystem(this));
}

void UFIVSProcessor::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	Runtime.PauseAndWait();
	RuntimeState.Clear();
	if (GetKernel()->GetState() != FIN_KERNEL_RUNNING) return;
	RuntimeState = Runtime.Runtime.SaveState();
	if (RuntimeState.IsFailure()) {
		FString message = FString::Printf(TEXT("%s: Unable to save computer state into a save-file (computer will restart when loading the save-file): %s"), *DebugInfo, *RuntimeState.Failure);
		UE_LOG(LogFicsItVisualScript, Display, TEXT("%s"), *message);
		GetKernel()->GetLog()->PushLogEntry(FIL_Verbosity_Warning, message);
	}
}

void UFIVSProcessor::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	Runtime.Runtime.Reset();
}

void UFIVSProcessor::GetRelevantObjects_Implementation(TArray<FFIRTrace>& OutObjects) {
	UObject* Component = GetKernel()->GetNetwork()->GetComponent().GetObject();
	for (UObject* Object : IFINNetworkCircuitNode::Execute_GetCircuit(Component)->GetComponents()) {
		OutObjects.Add(FFIRTrace(Component) / Object);
	}
}

void UFIVSProcessor::GetRelevantClasses_Implementation(TArray<UFIRClass*>& OutClasses) {
	for (const TPair<UClass*, UFIRClass*>& Class : FFicsItReflectionModule::Get().GetClasses()) {
		OutClasses.Add(Class.Value);
	}
}

void UFIVSProcessor::GetRelevantStructs_Implementation(TArray<UFIRStruct*>& OutStructs) {
	for (const TPair<UScriptStruct*, UFIRStruct*>& Struct : FFicsItReflectionModule::Get().GetStructs()) {
		OutStructs.Add(Struct.Value);
	}
}
