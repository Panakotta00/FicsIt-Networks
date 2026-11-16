#include "FIRHypertubeSubsystem.h"

#include "FGCharacterPlayer.h"
#include "FGPipeConnectionComponent.h"
#include "FGPlayerState.h"
#include "Buildables/FGBuildablePipeHyperJunction.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/SubsystemActorManager.h"

AFIRHypertubeSubsystem::AFIRHypertubeSubsystem() {
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
	bReplicates = true;
	bAlwaysRelevant = true;
}

AFIRHypertubeSubsystem* AFIRHypertubeSubsystem::Get(const UObject* WorldContext) {
#if WITH_EDITOR
	return nullptr;
#else
	if (!GEngine || !WorldContext) return nullptr;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;
	USubsystemActorManager* SubsystemActorManager = World->GetSubsystem<USubsystemActorManager>();
	if (!SubsystemActorManager) return nullptr;
	return SubsystemActorManager->GetSubsystemActor<AFIRHypertubeSubsystem>();
#endif
}

void AFIRHypertubeSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFIRHypertubeSubsystem, ForcedOutputs);
}

void AFIRHypertubeSubsystem::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	RemoveInvalidEntries();
}

void AFIRHypertubeSubsystem::SetForcedOutput(AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* Output, const FString& PlayerName) {
	if (!HasAuthority() || !IsValid(Junction)) return;

	RemoveInvalidEntries();
	ForcedOutputs.RemoveAll([Junction, &PlayerName](const FFIRHypertubeJunctionForce& Force) {
		return Force.Junction == Junction && Force.PlayerName == PlayerName;
	});
	if (IsValid(Output)) {
		ForcedOutputs.Add(FFIRHypertubeJunctionForce{Junction, Output, PlayerName});
	}

	ForceNetUpdate();
}

UFGPipeConnectionComponentBase* AFIRHypertubeSubsystem::GetForcedOutput(const AFGBuildablePipeHyperJunction* Junction, const FString& PlayerName) const {
	if (!Junction) return nullptr;
	for (const FFIRHypertubeJunctionForce& Force : ForcedOutputs) {
		if (Force.Junction == Junction && Force.PlayerName == PlayerName) {
			return IsValid(Force.Output) ? Force.Output.Get() : nullptr;
		}
	}
	return nullptr;
}

UFGPipeConnectionComponentBase* AFIRHypertubeSubsystem::GetForcedOutputForInput(const AFGBuildablePipeHyperJunction* Junction, const UFGPipeConnectionComponentBase* Input, const AFGCharacterPlayer* Player) const {
	if (!Junction || ForcedOutputs.Num() < 1) return nullptr;

	TArray<UFGPipeConnectionComponentBase*> Outputs;
	auto IsReachable = [&](UFGPipeConnectionComponentBase* Forced) {
		if (!Forced || Forced == Input) return false;
		if (Outputs.Num() < 1) Outputs = GetOutputsForInput(Junction, Input);
		return Outputs.Contains(Forced);
	};

	const FString PlayerName = GetPlayerName(Player);
	if (!PlayerName.IsEmpty()) {
		UFGPipeConnectionComponentBase* Forced = GetForcedOutput(Junction, PlayerName);
		if (IsReachable(Forced)) return Forced;
	}

	UFGPipeConnectionComponentBase* Forced = GetForcedOutput(Junction);
	if (IsReachable(Forced)) return Forced;
	return nullptr;
}

TArray<UFGPipeConnectionComponentBase*> AFIRHypertubeSubsystem::GetConnections(const AFGBuildablePipeHyperJunction* Junction) {
	TArray<UFGPipeConnectionComponentBase*> Connections;
	if (!IsValid(Junction)) return Connections;
	for (const FFGPipeHyperJunctionPath& Path : Junction->JunctionPaths) {
		if (IsValid(Path.Connection0)) Connections.AddUnique(Path.Connection0);
		if (IsValid(Path.Connection1)) Connections.AddUnique(Path.Connection1);
	}
	return Connections;
}

TArray<UFGPipeConnectionComponentBase*> AFIRHypertubeSubsystem::GetOutputsForInput(const AFGBuildablePipeHyperJunction* Junction, const UFGPipeConnectionComponentBase* Input) {
	TArray<UFGPipeConnectionComponentBase*> Outputs;
	if (!IsValid(Junction) || !Input) return Outputs;
	for (const FFGPipeHyperJunctionPath& Path : Junction->JunctionPaths) {
		if (Path.Connection0 == Input && IsValid(Path.Connection1)) Outputs.AddUnique(Path.Connection1);
		if (Path.Connection1 == Input && IsValid(Path.Connection0)) Outputs.AddUnique(Path.Connection0);
	}
	return Outputs;
}

UFGPipeConnectionComponentBase* AFIRHypertubeSubsystem::GetRouteOutput(const AFGBuildablePipeHyperJunction* Junction, int32 RouteIndex, const UFGPipeConnectionComponentBase* Input) {
	if (!IsValid(Junction) || !Junction->JunctionPaths.IsValidIndex(RouteIndex)) return nullptr;
	const FFGPipeHyperJunctionPath& Path = Junction->JunctionPaths[RouteIndex];
	if (Path.Connection0 == Input) return Path.Connection1;
	if (Path.Connection1 == Input) return Path.Connection0;
	return nullptr;
}

int32 AFIRHypertubeSubsystem::GetRouteIndex(const AFGBuildablePipeHyperJunction* Junction, const UFGPipeConnectionComponentBase* Input, const UFGPipeConnectionComponentBase* Output) {
	if (!IsValid(Junction) || !Input || !Output) return INDEX_NONE;
	for (int32 i = 0; i < Junction->JunctionPaths.Num(); ++i) {
		const FFGPipeHyperJunctionPath& Path = Junction->JunctionPaths[i];
		if ((Path.Connection0 == Input && Path.Connection1 == Output) || (Path.Connection1 == Input && Path.Connection0 == Output)) return i;
	}
	return INDEX_NONE;
}

FString AFIRHypertubeSubsystem::GetPlayerName(const AFGCharacterPlayer* Player) {
	if (!IsValid(Player)) return FString();
	const AFGPlayerState* PlayerState = Player->GetPlayerState<AFGPlayerState>();
	if (!IsValid(PlayerState)) return FString();
	return PlayerState->GetUserName();
}

void AFIRHypertubeSubsystem::RemoveInvalidEntries() {
	ForcedOutputs.RemoveAll([](const FFIRHypertubeJunctionForce& Force) {
		return !IsValid(Force.Junction) || !IsValid(Force.Output);
	});
}
