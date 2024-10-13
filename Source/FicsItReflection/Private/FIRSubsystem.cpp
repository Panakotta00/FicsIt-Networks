#include "FIRSubsystem.h"

#include "FGRailroadSubsystem.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "FGTrain.h"
#include "Buildables/FGBuildableRailroadSignal.h"
#include "Engine/Engine.h"
#include "Subsystem/SubsystemActorManager.h"

AFIRSubsystem* AFIRSubsystem::GetReflectionSubsystem(UObject* WorldContext) {
#if WITH_EDITOR
	return nullptr;
#endif
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFIRSubsystem>();
}

void AFIRSubsystem::BeginPlay() {
	Super::BeginPlay();

	for (TTuple<UFGRailroadTrackConnectionComponent*, FFIRRailroadSwitchForce>& Force : ForcedRailroadSwitches) {
		ForcedRailroadSwitchCleanup(Force.Value, Force.Key);
		UpdateRailroadSwitch(Force.Value, Force.Key);
	}
}

void AFIRSubsystem::ForceRailroadSwitch(UFGRailroadTrackConnectionComponent* RailroadSwitch, int64 Track) {
	FScopeLock ScopeLock(&ForcedRailroadSwitchesMutex);
	
	if (!IsValid(RailroadSwitch)) return;

	FFIRRailroadSwitchForce OldForce;
	if (ForcedRailroadSwitches.RemoveAndCopyValue(RailroadSwitch, OldForce)) {
		ForcedRailroadSwitchCleanup(OldForce, RailroadSwitch);
	}
	
	if (Track >= 0) {
		FFIRRailroadSwitchForce& Force = ForcedRailroadSwitches.Add(RailroadSwitch, FFIRRailroadSwitchForce{Track});
		UpdateRailroadSwitch(Force, RailroadSwitch);
	}
}

FFIRRailroadSwitchForce* AFIRSubsystem::GetForcedRailroadSwitch(UFGRailroadTrackConnectionComponent* RailroadSwitch) {
	FScopeLock ScopeLock(&ForcedRailroadSwitchesMutex);
	return ForcedRailroadSwitches.Find(RailroadSwitch);
}

void AFIRSubsystem::UpdateRailroadSwitch(FFIRRailroadSwitchForce& Force, UFGRailroadTrackConnectionComponent* Switch) {
	if (Force.ActualConnections.Num() == 0) {
		Force.ActualConnections = Switch->mConnectedComponents;
	}
	TArray<UFGRailroadTrackConnectionComponent*> Components = Switch->mConnectedComponents;
	for (UFGRailroadTrackConnectionComponent* Conn : Components) {
		if (!Conn) continue;
		Switch->RemoveConnectionInternal(Conn);
		//Conn->RemoveConnectionInternal(Switch);
	}
	if (Force.ActualConnections.Num() > Force.ForcedPosition) {
		UFGRailroadTrackConnectionComponent* ForcedTrack = Force.ActualConnections[Force.ForcedPosition];
		if (ForcedTrack) {
			Switch->AddConnectionInternal(ForcedTrack);
			//ForcedTrack->AddConnectionInternal(Switch);
		}
	}

	for (AFGRailroadVehicle* vehicle : Switch->GetTrack()->GetVehicles()) {
		vehicle->GetTrain()->mAtcData.ClearPath();
	}
	
	TWeakPtr<FFGRailroadSignalBlock> Block = Switch->GetConnections().Num() > 0 ? Switch->GetConnections()[0]->GetSignalBlock() : nullptr;
	AFGRailroadSubsystem::Get(Switch)->RebuildSignalBlocks(Switch->GetTrack()->GetTrackGraphID());
	if (AFGBuildableRailroadSignal* FacingSignal = Switch->GetFacingSignal()) {
		//FacingSignal->SetObservedBlock(Block);
		FacingSignal->UpdateConnections();
	}
}

void AFIRSubsystem::AddRailroadSwitchConnection(CallScope<void(*)(UFGRailroadTrackConnectionComponent*,UFGRailroadTrackConnectionComponent*)>& Scope, UFGRailroadTrackConnectionComponent* Switch, UFGRailroadTrackConnectionComponent* Connection) {
	FScopeLock ScopeLock(&ForcedRailroadSwitchesMutex);
	FFIRRailroadSwitchForce* ForcedTrack = GetForcedRailroadSwitch(Switch);
	if (ForcedTrack) {
		ForcedTrack->ActualConnections.Add(Connection);
		Connection->RemoveConnectionInternal(Switch);
	}
}

void AFIRSubsystem::RemoveRailroadSwitchConnection(CallScope<void(*)(UFGRailroadTrackConnectionComponent*, UFGRailroadTrackConnectionComponent*)>& Scope, UFGRailroadTrackConnectionComponent* Switch, UFGRailroadTrackConnectionComponent* Connection) {
	FScopeLock ScopeLock(&ForcedRailroadSwitchesMutex);
	FFIRRailroadSwitchForce* ForcedTrack = GetForcedRailroadSwitch(Switch);
	if (ForcedTrack) {
		Switch->RemoveConnectionInternal(Connection);
		ForcedTrack->ActualConnections.Remove(Connection);
	}
}

TOptional<TTuple<FCriticalSection&, FFIRFactoryConnectorSettings&>> AFIRSubsystem::GetFactoryConnectorSettings(UFGFactoryConnectionComponent* InConnector) {
	FactoryConnectorSettingsMutex.Lock();
	FFIRFactoryConnectorSettings* Settings = FactoryConnectorSettings.Find(InConnector);
	if (Settings) {
		return TOptional(TTuple<FCriticalSection&, FFIRFactoryConnectorSettings&>(FactoryConnectorSettingsMutex, *Settings));
	} else {
		FactoryConnectorSettingsMutex.Unlock();
		return {};
	}
}

void AFIRSubsystem::SetFactoryConnectorAllowedItem(UFGFactoryConnectionComponent* InConnector, TSubclassOf<UFGItemDescriptor> InAllowedItem) {
	FScopeLock ScopeLock(&FactoryConnectorSettingsMutex);
	FFIRFactoryConnectorSettings* Settings = nullptr;
	if (InAllowedItem != nullptr) {
		Settings = &FactoryConnectorSettings.FindOrAdd(InConnector);
	} else {
		Settings = FactoryConnectorSettings.Find(InConnector);
	}
	if (Settings) {
		Settings->AllowedItem = InAllowedItem;
		FactoryConnectorCleanup(InConnector, *Settings);
	}
}

TSubclassOf<UFGItemDescriptor> AFIRSubsystem::GetFactoryConnectorAllowedItem(UFGFactoryConnectionComponent* InConnector) {
	FScopeLock ScopeLock(&FactoryConnectorSettingsMutex);
	FFIRFactoryConnectorSettings* Settings = FactoryConnectorSettings.Find(InConnector);
	if (Settings) {
		return Settings->AllowedItem;
	} else {
		return nullptr;
	}
}

void AFIRSubsystem::SetFactoryConnectorBlocked(UFGFactoryConnectionComponent* InConnector, bool bInBlocked) {
	FScopeLock ScopeLock(&FactoryConnectorSettingsMutex);
	FFIRFactoryConnectorSettings* Settings = nullptr;
	if (bInBlocked) {
		Settings = &FactoryConnectorSettings.FindOrAdd(InConnector);
	} else {
		Settings = FactoryConnectorSettings.Find(InConnector);
	}
	if (Settings) {
		Settings->bBlocked = bInBlocked;
		FactoryConnectorCleanup(InConnector, *Settings);
	}
}

bool AFIRSubsystem::GetFactoryConnectorBlocked(UFGFactoryConnectionComponent* InConnector) {
	FScopeLock ScopeLock(&FactoryConnectorSettingsMutex);
	FFIRFactoryConnectorSettings* Settings = FactoryConnectorSettings.Find(InConnector);
	return Settings ? Settings->bBlocked : false;
}

int64 AFIRSubsystem::AddFactoryConnectorUnblockedTransfers(UFGFactoryConnectionComponent* InConnector, int64 InUnblockedTransfers) {
	FScopeLock ScopeLock(&FactoryConnectorSettingsMutex);
	FFIRFactoryConnectorSettings* Settings = FactoryConnectorSettings.Find(InConnector);
	if (Settings) {
		Settings->UnblockedTransfers = FMath::Max(0, Settings->UnblockedTransfers + InUnblockedTransfers);
		return Settings->UnblockedTransfers;
	} else {
		return 0;
	}
}

int64 AFIRSubsystem::GetFactoryConnectorUnblockedTransfers(UFGFactoryConnectionComponent* InConnector) {
	FScopeLock ScopeLock(&FactoryConnectorSettingsMutex);
	FFIRFactoryConnectorSettings* Settings = FactoryConnectorSettings.Find(InConnector);
	if (Settings) {
		return Settings->UnblockedTransfers;
	} else {
		return 0;
	}
}

void AFIRSubsystem::ForcedRailroadSwitchCleanup(FFIRRailroadSwitchForce& Force, UFGRailroadTrackConnectionComponent* Switch) {
	TArray<UFGRailroadTrackConnectionComponent*> Components = Switch->mConnectedComponents;
	for (UFGRailroadTrackConnectionComponent* Conn : Components) {
		if (!Conn) continue;
		Switch->RemoveConnectionInternal(Conn);
		Conn->RemoveConnectionInternal(Switch);
	}
	for (UFGRailroadTrackConnectionComponent* Conn : Force.ActualConnections) {
		if (!Conn) continue;
		Switch->AddConnectionInternal(Conn);
		Conn->AddConnectionInternal(Switch);
	}
	TWeakPtr<FFGRailroadSignalBlock> Block = Switch->GetConnections().Num() > 0 ? Switch->GetConnections()[0]->GetSignalBlock() : nullptr;
	AFGRailroadSubsystem::Get(Switch)->RebuildSignalBlocks(Switch->GetTrack()->GetTrackGraphID());
	if (AFGBuildableRailroadSignal* FacingSignal = Switch->GetFacingSignal()) {
		//FacingSignal->SetObservedBlock(Block);
		FacingSignal->UpdateConnections();
	}
}

void AFIRSubsystem::FactoryConnectorCleanup(UFGFactoryConnectionComponent* InConnector, FFIRFactoryConnectorSettings& Settings) {
	if (Settings.bBlocked || Settings.AllowedItem != nullptr) return;
	FactoryConnectorSettings.Remove(InConnector);
}
