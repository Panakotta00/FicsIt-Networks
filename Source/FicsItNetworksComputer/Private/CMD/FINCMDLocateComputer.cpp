#include "CoreMinimal.h"
#include "AkGameplayStatics.h"
#include "FGAttentionPingActor.h"
#include "FGPlayerController.h"
#include "FicsItNetworksComputer.h"
#include "FINComputerCase.h"
#include "FINNetworkComponent.h"
#include "FINNetworkUtils.h"
#include "Kismet/GameplayStatics.h"

bool ExecCMDLocateComputer(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
	if (FParse::Command(&Command, TEXT("FINLocateComputer"))) {
		FString computerStr = FParse::Token(Command, true);

		TArray<AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass(World, AFINComputerCase::StaticClass(), actors);

		AFINComputerCase* computer = nullptr;
		FGuid uuid;
		if (FGuid::Parse(computerStr, uuid)) {
			for (AActor* actor : actors) {
				UObject* networkComponent = UFINNetworkUtils::FindNetworkComponentFromObject(actor);
				if (IFINNetworkComponent::Execute_GetID(networkComponent) == uuid) {
					computer = Cast<AFINComputerCase>(actor);
					break;
				}
			}
		} else {
			for (AActor* actor : actors) {
				if (actor->GetName() == computerStr) {
					computer = Cast<AFINComputerCase>(actor);
					break;
				}
			}
		}
		if (computer == nullptr) {
			UE_LOG(LogFicsItNetworksComputer, Display, TEXT("Unable to locate FicsIt-Networks Computer '%s'"), *computerStr);
			return true;
		}

		FVector Position = computer->GetActorLocation();
		UE_LOG(LogFicsItNetworksComputer, Display, TEXT("Located FicsIt-Networks Computer '%s' at: %f %f %f"), *computerStr, Position.X, Position.Y, Position.Z);

		for (auto players = World->GetPlayerControllerIterator(); players; ++players) {
			AFGPlayerController* PlayerController = Cast<AFGPlayerController>(players->Get());
			UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Player/BP_AttentionPingActor.BP_AttentionPingActor_C"));
			AFGAttentionPingActor* PingActor = PlayerController->GetWorld()->SpawnActorDeferred<AFGAttentionPingActor>(Class, FTransform(Position));
			PingActor->SetOwningPlayerState(PlayerController->GetPlayerState<AFGPlayerState>());
			PingActor->FinishSpawning(FTransform(Position));
		}

		return true;
	}
	return false;
}

[[maybe_unused]] static FStaticSelfRegisteringExec SelfRegisterCMDLocateComputer(&ExecCMDLocateComputer);