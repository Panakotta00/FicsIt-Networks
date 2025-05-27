#pragma once

#include "CoreMinimal.h"
#include "FGAvailabilityDependency.h"
#include "FGSaveInterface.h"
#include "ModSubsystem.h"
#include "Engine/DataAsset.h"
#include "FINChallengeSubsystem.generated.h"

#define FINChallenge(Name, Condition) \
	static TSharedRef<FFINChallenge> finChallenge_ ## Name = AFINChallengeSubsystem::RegisterChallenge(TEXT(#Name)); \
	if (!finChallenge_ ## Name->bCompleted) { \
		if (Condition) { \
			finChallenge_ ## Name->Complete(); \
		} \
	}

/**
 * On construction, registers itself to the FINChallengeSubsystem.
 * It represents a challenge with a name which should be unique.
 * It provides a trampoline to quickly and efficiently check if the challenge is completed
 * and allows marking it as completed.
 */
struct FICSITNETWORKSMISC_API FFINChallenge : TSharedFromThis<FFINChallenge> {
	FString Name;

	/**
	 * True if the challenge has been completed.
	 * This is not the source of truth! That would be the AFINChallengeSubsystem.
	 * This is only intended as optimization for often executed code.
	 */
	bool bCompleted = false;

	TSoftObjectPtr<AFINChallengeSubsystem> Subsystem;

	FFINChallenge(const FString& Name) : Name(Name) {}
	~FFINChallenge();

	void Complete();
};

UCLASS()
class FICSITNETWORKSMISC_API UFINChallengeAsset : public UDataAsset {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
	FString Name;

	virtual void Serialize(FArchive& Ar) override;

private:
	TSharedPtr<FFINChallenge> Challenge;
};

UCLASS()
class FICSITNETWORKSMISC_API AFINChallengeSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
public:
	AFINChallengeSubsystem();

	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
 	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	bool IsChallengeCompleted(const FString& Name) {
		return CompletedChallenges.Contains(Name);
	}

	void CompleteChallenge(const FString& Name) {
		CompletedChallenges.Add(Name);
	}

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContext"))
	static AFINChallengeSubsystem* GetSubsystem(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, DisplayName="Complete Challenge", meta=(WorldContext="WorldContext"))
	static void K_CompleteChallenge(UObject* WorldContext, UFINChallengeAsset* Challenge);

	static TSharedRef<FFINChallenge> RegisterChallenge(const FString& ChallengeName);
	static void UnregisterChallenge(const FString& ChallengeName);

private:
	static TSoftObjectPtr<AFINChallengeSubsystem> self;

	UPROPERTY(SaveGame, Replicated)
	TArray<FString> CompletedChallenges;

	static TMap<FString, TWeakPtr<FFINChallenge>> Challenges;
};

UCLASS(Blueprintable)
class FICSITNETWORKSMISC_API UFINChallengeDependency : public UFGAvailabilityDependency {
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Dependency")
	FString ChallengeName;

	UPROPERTY(EditDefaultsOnly, Category="Dependency")
	FText Description;

	// Begin UFGAvailabilityDependency
	virtual bool AreDependenciesMet(UObject* worldContext) const override;
	// End UFGAvailabilityDependency

	UFUNCTION(BlueprintCallable)
	virtual FText GetDescription() const { return Description; }
};