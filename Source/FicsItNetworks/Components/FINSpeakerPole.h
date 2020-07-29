#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Network/FINFuture.h"
#include "Network/FINNetworkCustomType.h"


#include "FINSpeakerPole.generated.h"

class AFINSpeakerPole;

USTRUCT()
struct FFINSpeakersPlaySoundFuture : public FFINFutureSimpleDone {
	GENERATED_BODY()

	FFINSpeakersPlaySoundFuture() = default;
	FFINSpeakersPlaySoundFuture(AFINSpeakerPole* Speakers, const FString& Sound, float Start) : Speakers(Speakers), Sound(Sound), Start(Start) {}
	
	UPROPERTY(SaveGame)
	AFINSpeakerPole* Speakers = nullptr;

	UPROPERTY(SaveGame)
	FString Sound = "";

	UPROPERTY(SaveGame)
	float Start = 0.0f;

	virtual void Execute() override;
	virtual int operator>>(FFINValueReader& Reader) const override { return 0; }
};

USTRUCT()
struct FFINSpeakersStopSoundFuture : public FFINFutureSimpleDone {
	GENERATED_BODY()

	FFINSpeakersStopSoundFuture() = default;
	FFINSpeakersStopSoundFuture(AFINSpeakerPole* Speakers) : Speakers(Speakers) {}
	
	UPROPERTY(SaveGame)
    AFINSpeakerPole* Speakers = nullptr;

	virtual void Execute() override;
	virtual int operator>>(FFINValueReader& Reader) const override { return 0; }
};

UCLASS(Blueprintable)
class AFINSpeakerPole : public AFGBuildable, public IFINSignalSender, public IFINNetworkCustomType {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
	UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
	UAudioComponent* AudioComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="SpeakerPole")
	FString CurrentSound;

	UPROPERTY(SaveGame)
	TSet<FFINNetworkTrace> Listeners;

	AFINSpeakerPole();

	// Begin IFINNetworkSignalSender
	virtual void AddListener_Implementation(FFINNetworkTrace listener) override;
	virtual void RemoveListener_Implementation(FFINNetworkTrace listener) override;
	virtual TSet<FFINNetworkTrace> GetListeners_Implementation() override;
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINNetworkSignalSender

	// Begin IFINNetworkCustomType
	virtual FString GetCustomTypeName_Implementation() const override { return TEXT("SpeakerPole"); }
	// End IFINNetworkCustomType

	/**
	 * Event bound to the OnAudioFinished event of the AudioComponent.
	 * Triggers a network signal notifyng that the audio has stoped playing.
	 * Also resets CurrentSound
	 */
	UFUNCTION()
	void OnSoundFinished(UAudioComponent* Audio);

	/**
	 * Loads and Plays the sound file in the Sounds folder appened with the given relative path without the file extension.
	 * Plays the sound at the given startPoint.
	 * Might cause the current sound playing to stop even if the new sound is not found.
	 * If able to play the sound, emits a play sound signal.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Component")
	FFINSpeakersPlaySoundFuture netFunc_playSound(const FString& sound, float startPoint);

	/**
	 * Stops the current playing sound.
	 * Emits a stop sound signal if it actually was able to stop the current playing sound.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Component")
	FFINDynamicStructHolder netFunc_stopSound();

	/**
	 * Notifies when the state of the speaker pole has changed.
	 * f.e. if the sound stoped/started playing
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_SpeakerSound(int type, const FString& sound);

	/**
	 * Loads the sound file referenced by the given relative path
	 * from the %localappdata%/FactoryGame/Saved/SaveGames/Computers/Sounds folder
	 * without the file extension.
	 * If it was unable to load the sound file, return nullptr.
	 */
	USoundWave* LoadSoundFromFile(const FString& sound);
};
