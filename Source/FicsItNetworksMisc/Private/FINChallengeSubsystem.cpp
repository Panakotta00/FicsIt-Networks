#include "FINChallengeSubsystem.h"

#include "SubsystemActorManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

FFINChallenge::~FFINChallenge() {
	AFINChallengeSubsystem::UnregisterChallenge(Name);
}

void FFINChallenge::Complete() {
	if (auto subsys = Subsystem.Get()) {
		subsys->CompleteChallenge(Name);
	}
}

void UFINChallengeAsset::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);

#if !WITH_EDITOR
	Challenge = AFINChallengeSubsystem::RegisterChallenge(Name);
#endif
}

TSoftObjectPtr<AFINChallengeSubsystem> AFINChallengeSubsystem::self;
TMap<FString, TWeakPtr<FFINChallenge>> AFINChallengeSubsystem::Challenges;

void AFINChallengeSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFINChallengeSubsystem, CompletedChallenges);
}

AFINChallengeSubsystem::AFINChallengeSubsystem() {
	bReplicates = true;
}

void AFINChallengeSubsystem::BeginPlay() {
	Super::BeginPlay();

	self = this;

	for (auto challenge : Challenges) {
		if (auto c = challenge.Value.Pin()) {
			c->Subsystem = this;
			c->bCompleted = IsChallengeCompleted(challenge.Key);
		}
	}
}

void AFINChallengeSubsystem::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	for (auto challenge : Challenges) {
		if (auto c = challenge.Value.Pin()) {
			c->bCompleted = IsChallengeCompleted(challenge.Key);
		}
	}
}

AFINChallengeSubsystem* AFINChallengeSubsystem::GetSubsystem(UObject* WorldContext) {
	return WorldContext->GetWorld()->GetSubsystem<USubsystemActorManager>()->GetSubsystemActor<AFINChallengeSubsystem>();
}

void AFINChallengeSubsystem::K_CompleteChallenge(UObject* WorldContext, UFINChallengeAsset* Challenge) {
	AFINChallengeSubsystem::GetSubsystem(WorldContext)->CompleteChallenge(Challenge->Name);
}

TSharedRef<FFINChallenge> AFINChallengeSubsystem::RegisterChallenge(const FString& ChallengeName) {
	TSharedPtr<FFINChallenge> challenge;
	if (auto* challengeWeak = Challenges.Find(ChallengeName)) {
		challenge = challengeWeak->Pin();
	}
	if (!challenge) {
		challenge = MakeShareable(new FFINChallenge(ChallengeName));
		Challenges.Add(challenge->Name,	challenge);
	}


	if (AFINChallengeSubsystem* subsys = self.Get()) {
		challenge->Subsystem = subsys;
		challenge->bCompleted = subsys->IsChallengeCompleted(challenge->Name);
	}

	return challenge.ToSharedRef();
}

void AFINChallengeSubsystem::UnregisterChallenge(const FString& ChallengeName) {
	Challenges.Remove(ChallengeName);
}

bool UFINChallengeDependency::AreDependenciesMet(UObject* worldContext) const {
	auto manager = worldContext->GetWorld()->GetSubsystem<USubsystemActorManager>();
	if (!IsValid(manager)) return false;
	auto subsys = manager->GetSubsystemActor<AFINChallengeSubsystem>();
	if (!IsValid(subsys)) return false;
	return subsys->IsChallengeCompleted(ChallengeName);
}
