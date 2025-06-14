#include "FINMicrocontroller.h"

#include "FicsItNetworksLuaModule.h"
#include "FINComputerEEPROMDesc.h"
#include "FINItemStateEEPROMText.h"
#include "FINMicrocontrollerReference.h"
#include "LuaUtil.h"
#include "Async.h"
#include "FicsItNetworksMicrocontroller.h"
#include "FicsItReflection.h"
#include "FILLogContainer.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINComputerNetworkCard.h"
#include "FINMicrocontrollerLuaModule.h"
#include "FINNetworkUtils.h"
#include "FINSignalSubsystem.h"
#include "LuaComponentAPI.h"
#include "LuaEventAPI.h"
#include "LuaFuture.h"
#include "LuaWorldAPI.h"
#include "NetworkController.h"

AFINMicrocontroller::AFINMicrocontroller() {
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Inventory = CreateDefaultSubobject<UFGInventoryComponent>(TEXT("Inventory"));
	Inventory->SetDefaultSize(1);
	Inventory->Resize(1);

	Log = CreateDefaultSubobject<UFILLogContainer>("Log");

	NetworkController = CreateDefaultSubobject<UFINKernelNetworkController>("NetworkController");

	SetupRuntime();
}

AFINMicrocontroller::~AFINMicrocontroller() {
	Destroy();
}

void AFINMicrocontroller::BeginPlay() {
	Super::BeginPlay();

	if (!ID.IsValid()) {
		ID = FGuid::NewGuid();
	}

	if (!IsValid(NetworkComponent)) {
		TArray<FInventoryStack> refund;
		Execute_GetDismantleRefund(this, refund, false);
		FDismantleHelpers::DropRefundOnGround(this, GetActorLocation(), refund, nullptr);
		Execute_Dismantle(this);
	} else {
		Reference = Cast<UFINMicrocontrollerReference>(NetworkComponent->AddComponentByClass(UFINMicrocontrollerReference::StaticClass(), false, FTransform::Identity, true));
		Reference->Microcontroller = this;
		NetworkComponent->FinishAddComponent(Reference, false, FTransform::Identity);
		UObject* net = UFINNetworkUtils::FindNetworkComponentFromObject(NetworkComponent);

		AFINSignalSubsystem::GetSignalSubsystem(this)->Listen(this, FFIRTrace(this));

		if (UFINAdvancedNetworkConnectionComponent* connector = Cast<UFINAdvancedNetworkConnectionComponent>(net)) {
			NetworkController->SetComponent(this);
			connector->AddConnectedNode(this);
		} else {
			NetworkController->SetComponent(net);
		}
	}
}

void AFINMicrocontroller::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	HandledMessages.Empty();

	if (GetStatus() != FIN_Microcontroller_State_Running) return;

	/*if (GetNetwork()->GetSignalCount() > 0) {
		Runtime.Timeout.Reset();
	}*/

	Runtime.Hook_Tick = 500;
	Runtime.Tick();

	switch (Runtime.GetStatus()) {
		case FFINLuaRuntime::Finished:
			StopRuntime();
			break;
		case FFINLuaRuntime::Crashed:
			if (TOptional<FString> error = Runtime.GetError()) {
				CrashRuntime(*error);
			}
			break;
		default: break;
	}
}

void AFINMicrocontroller::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	PersistenceState.Clear();
	if (GetStatus() != FIN_Microcontroller_State_Running) return;
	PersistenceState = Runtime.SaveState();
	if (PersistenceState.IsFailure()) {
		FString message = FString::Printf(TEXT("%s: Unable to save computer state into a save-file (computer will restart when loading the save-file): %s"), *GetDebugInfo(), *PersistenceState.Failure);
		UE_LOG(LogFicsItNetworksMicrocontroller, Display, TEXT("%s"), *message);
		GetLog()->PushLogEntry(FIL_Verbosity_Warning, message);
	}
}

void AFINMicrocontroller::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	if (PersistenceState.LuaData.Len() <= 0) return;
	StartRuntime();
}

void AFINMicrocontroller::HandleSignal(const FFINSignalData& Signal, const FFIRTrace& Sender) {
	NetworkController->HandleSignal(Signal, Sender);
	Runtime.Timeout.Reset();
}

TSet<UObject*> AFINMicrocontroller::GetConnected_Implementation() const {
	return {UFINNetworkUtils::FindNetworkComponentFromObject(NetworkComponent)};
}

AFINNetworkCircuit* AFINMicrocontroller::GetCircuit_Implementation() const {
	return NetworkCircuit;
}

void AFINMicrocontroller::SetCircuit_Implementation(AFINNetworkCircuit* Circuit) {
	NetworkCircuit = Circuit;
}

void AFINMicrocontroller::HandleMessage(const FGuid& MsgID, const FGuid& Sender, const FGuid& Receiver, int Port, const FIRArray& Data) {
	static UFIRSignal* Signal = nullptr;
	if (!Signal) Signal = FFicsItReflectionModule::Get().FindClass(StaticClass())->FindFIRSignal("NetworkMessage");
	{
		FScopeLock Lock(&HandledMessagesMutex);
		if (HandledMessages.Contains(MsgID) || !Signal) return;
		HandledMessages.Add(MsgID);
	}
	if (!OpenPorts.Contains(Port)) return;
	if (Receiver.IsValid() && Receiver != ID) return;
	TArray<FFIRAnyValue> Parameters = { Sender.ToString(), (FIRInt)Port };
	Parameters.Append(Data);
	Signal->Trigger(this, Parameters);
}

void AFINMicrocontroller::ToggleRuntime() {
	switch (Runtime.GetStatus()) {
		case FFINLuaRuntime::Running:
			StopRuntime();
			break;
		default:
			Log->EmptyLog();
			StartRuntime();
	}
}

void AFINMicrocontroller::StartRuntime() {
	StopRuntime();

	PersistenceState.Clear();
	Runtime.Reset();
	TOptional<FString> Code = GetCode();
	if (Code) {
		TOptional<FString> error = Runtime.LoadCode(*Code);
		if (error) {
			CrashRuntime(*error);
		}
	} else {
		CrashRuntime(TEXT("Failed to start Microcontroller: No EEPROM Present"));
	}
}

void AFINMicrocontroller::StopRuntime() {
	Runtime.Destroy();
	Error.Empty();
	Proxies.Empty();
	OpenPorts.Empty();
	AFINSignalSubsystem* Subsys = AFINSignalSubsystem::GetSignalSubsystem(this);
	if (Subsys) Subsys->IgnoreAll(this);
	NetworkController->ClearSignals();
}

void AFINMicrocontroller::CrashRuntime(const FString& message) {
	Error = message;
	GetLog()->PushLogEntry(FIL_Verbosity_Fatal, message);
	Runtime.Destroy();
}

TOptional<FString> AFINMicrocontroller::GetCode() const {
	FInventoryStack stack;
	Inventory->GetStackFromIndex(0, stack);
	if (const FFINItemStateEEPROMText* eeprom = stack.Item.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		return eeprom->Code;
	}
	return {};
}

FString AFINMicrocontroller::GetCode(const FString& Default) const {
	TOptional<FString> code = GetCode();
	if (code) return *code;
	return Default;
}

void AFINMicrocontroller::SetCode(const FString& Code) {
	FInventoryStack stack;
	if (!Inventory->GetStackFromIndex(0, stack) || !stack.HasItems()) return;

	AsyncTask(ENamedThreads::GameThread, [this, stack, Code]() {
		FInventoryItem item = stack.Item;
		UFINComputerEEPROMDesc::CreateEEPROMStateInItem(item);
		if (const FFINItemStateEEPROMText* state = item.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
			FFINItemStateEEPROMText newState = *state;
			newState.Code = Code;
			Inventory->SetStateOnIndex(0, FFGDynamicStruct(newState));
			Inventory->OnSlotUpdatedDelegate.Broadcast(0);
		}
	});
}

void AFINMicrocontroller::SetStorage(const FString& InStorage) {
	Storage = InStorage;
	OnStorageChanged.Broadcast(Storage);
}
FString AFINMicrocontroller::GetStorage() const {
	return Storage;
}

FString AFINMicrocontroller::GetDebugInfo() const {
	return GetName();
}

UFILLogContainer* AFINMicrocontroller::GetLog() const {
	return Log;
}

EFINMicrocontrollerState AFINMicrocontroller::GetStatus() const {
	if (!Error.IsEmpty()) return FIN_Microcontroller_State_Failed;

	switch (Runtime.GetStatus()) {
		case FFINLuaRuntime::Running:
			return FIN_Microcontroller_State_Running;
		case FFINLuaRuntime::Crashed:
			return FIN_Microcontroller_State_Failed;
		default:
			return FIN_Microcontroller_State_Stopped;
	}
}

void AFINMicrocontroller::SetupRuntime() {
	Runtime.Modules.Add("DebugModule");
	Runtime.Modules.Add("WorldModule");
	Runtime.Modules.Add("MicrocontrollerModule");
	Runtime.Modules.Add("FutureModule");
	Runtime.Modules.Add("LogModule");
	Runtime.Modules.Add("ComponentModule");
	Runtime.Modules.Add("EventModule");

	Runtime.OnPreModules.AddWeakLambda(this, [this]() {
		lua_State* L = Runtime.GetLuaState();
		FINLua::luaFIN_setReferenceCollector(L, ReferenceCollector);
		FINLua::luaFIN_setWorld(L, GetWorld());
		FINLua::luaFIN_setMicrocontroller(L, this);
		FINLua::luaFIN_setComponentNetwork(L, &ComponentNetwork);
		FINLua::luaFIN_setEventSystem(L, EventSystem);
		FINLua::luaFIN_createFutureDelegate(L).AddWeakLambda(this, [this](const FINLua::FLuaFuture& Future) {
			FINLua::FLuaFuture f = Future;
			AsyncTask(ENamedThreads::GameThread, [this, f]() { // TODO: Maybe this has to be a soft object ptr
				try {
					(*f)->Execute();
				} catch (FFIRException e) {
					CrashRuntime(e.GetMessage()); // TODO: Maybe add a way to make these future crashes catchable in f.e. Lua using protected calls
				}
			});
		});
	});

	Runtime.OnPreLuaTick.AddWeakLambda(this, [this](TArray<TSharedPtr<void>>& TickStack) {
		TickStack.Add(MakeShared<FFILLogScope>(GetLog()));
	});
	Runtime.OnPostReset.AddWeakLambda(this, [this]() {
		TOptional<FString> error = Runtime.LoadState(PersistenceState);
		if (error) {
			StartRuntime();
			FString message = FString::Printf(TEXT("%s: Unable to load computer state from save-file (computer will restart): %s"), *GetDebugInfo(), **error);
			UE_LOG(LogFicsItNetworksMicrocontroller, Display, TEXT("%s"), *message);
			GetLog()->PushLogEntry(FIL_Verbosity_Warning, message);
		}
	});

	ComponentNetwork.OnGetComponentByID.BindWeakLambda(this, [this](const FGuid& ID) {
		if (Proxies.Num() >= ProxyLimit && !Proxies.Contains(ID)) throw FFINLuaPanic{TEXT("Unable to proxy more components!")};
		FFIRTrace trace = NetworkController->GetComponentByID(ID);
		if (trace) Proxies.Add(ID);
		return trace;
	});
	ComponentNetwork.OnGetComponentByNick.BindWeakLambda(this, [this](const FString& Nick) {
		return NetworkController->GetComponentByNick(Nick);
	});
	ComponentNetwork.OnGetComponentByClass.BindWeakLambda(this, [this](UClass* Class, bool bInRedirect) {
		return NetworkController->GetComponentByClass(Class, bInRedirect);
	});
	EventSystem.OnListen.BindWeakLambda(this, [this](FFIRTrace Object) {
		AFINSignalSubsystem::GetSignalSubsystem(this)->Listen(Object.GetUnderlyingPtr(), Object.Reverse() / NetworkController->GetComponent().GetObject() / this);
	});
	EventSystem.OnListening.BindWeakLambda(this, [this]() {
		return AFINSignalSubsystem::GetSignalSubsystem(this)->GetListening(this);
	});
	EventSystem.OnIgnoreAll.BindWeakLambda(this, [this]() {
		AFINSignalSubsystem::GetSignalSubsystem(this)->IgnoreAll(this);
	});
	EventSystem.OnClear.BindWeakLambda(this, [this]() {
		NetworkController->ClearSignals();
	});
	EventSystem.OnIgnore.BindWeakLambda(this, [this](UObject* Object) {
		AFINSignalSubsystem::GetSignalSubsystem(this)->Ignore(this, Object);
	});
	EventSystem.OnPullSignal.BindWeakLambda(this, [this]() -> TOptional<TTuple<FFIRTrace, FFINSignalData>> {
		if (NetworkController->GetSignalCount() < 1) {
			return {};
		}
		FFIRTrace sender;
		FFINSignalData data = NetworkController->PopSignal(sender);
		return {{sender, data}};
	});
}
