#include "FINChallengeSubsystem.h"

#include "SubsystemActorManager.h"
#include "Engine/World.h"

FFINChallenge::~FFINChallenge() {
	AFINChallengeSubsystem::UnregisterChallenge(Name);
}

void FFINChallenge::Complete() {
	if (auto subsys = Subsystem.Get()) {
		subsys->CompleteChallenge(Name);
	}
}

TSoftObjectPtr<AFINChallengeSubsystem> AFINChallengeSubsystem::self;
TMap<FString, TWeakPtr<FFINChallenge>> AFINChallengeSubsystem::Challenges;

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
	auto subsys = worldContext->GetWorld()->GetSubsystem<USubsystemActorManager>()->GetSubsystemActor<AFINChallengeSubsystem>();
	return subsys->IsChallengeCompleted(ChallengeName);
}
