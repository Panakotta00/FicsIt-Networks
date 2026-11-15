#include "FIRHypertubeSubsystem.h"

#include "Buildables/FGBuildablePipeHyperJunction.h"
#include "FGPipeConnectionComponent.h"
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

void AFIRHypertubeSubsystem::SetForcedOutput(AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* Output) {
	if (!HasAuthority() || !IsValid(Junction)) return;

	RemoveInvalidEntries();
	ForcedOutputs.RemoveAll([Junction](const FFIRHypertubeJunctionForce& Force) {
		return Force.Junction == Junction;
	});
	if (IsValid(Output)) {
		ForcedOutputs.Add(FFIRHypertubeJunctionForce{Junction, Output});
	}

	ForceNetUpdate();
}

UFGPipeConnectionComponentBase* AFIRHypertubeSubsystem::GetForcedOutput(const AFGBuildablePipeHyperJunction* Junction) const {
	if (!Junction) return nullptr;
	for (const FFIRHypertubeJunctionForce& Force : ForcedOutputs) {
		if (Force.Junction == Junction) {
			return IsValid(Force.Output) ? Force.Output.Get() : nullptr;
		}
	}
	return nullptr;
}

UFGPipeConnectionComponentBase* AFIRHypertubeSubsystem::GetForcedOutputForInput(const AFGBuildablePipeHyperJunction* Junction, const UFGPipeConnectionComponentBase* Input) const {
	UFGPipeConnectionComponentBase* Forced = GetForcedOutput(Junction);
	if (!Forced || Forced == Input) return nullptr;
	if (!GetOutputsForInput(Junction, Input).Contains(Forced)) return nullptr;
	return Forced;
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

void AFIRHypertubeSubsystem::RemoveInvalidEntries() {
	ForcedOutputs.RemoveAll([](const FFIRHypertubeJunctionForce& Force) {
		return !IsValid(Force.Junction) || !IsValid(Force.Output);
	});
}
