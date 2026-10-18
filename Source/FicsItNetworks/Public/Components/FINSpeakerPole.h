#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "Engine/TimerHandle.h"
#include "Signals/FINSignalSender.h"
#include "FINSpeakerPole.generated.h"

/**
 * Sound currently played by a speaker pole, replicated so clients (also late joining ones) play it in sync.
 */
USTRUCT()
struct FFINSpeakerPlayback {
	GENERATED_BODY()

	/** Sound file path relative to the sounds folder, without the file extension */
	UPROPERTY()
	FString Sound;

	/** MD5 hash of the sound file of the server, empty if nothing is playing */
	UPROPERTY()
	FString Hash;

	/** Size of the sound file in bytes */
	UPROPERTY()
	int32 Size = 0;

	/** Start point in seconds */
	UPROPERTY()
	float StartPoint = 0.f;

	/** Length of the sound in seconds */
	UPROPERTY()
	float Duration = 0.f;

	/** Server world time at which the sound started playing */
	UPROPERTY()
	double ServerStartTime = 0.0;

	bool IsSet() const { return !Hash.IsEmpty(); }
};

UCLASS(Blueprintable)
class AFINSpeakerPole : public AFGBuildable, public IFINSignalSender {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
	class UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
	class UAudioComponent* AudioComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="SpeakerPole")
	FString CurrentSound;

	AFINSpeakerPole();

	// Begin AActor
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End AActor

	// Begin IFINNetworkSignalSender
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINNetworkSignalSender

	/**
	 * Plays the given sound file of the server's sounds folder on the server and all clients.
	 * Clients which don't have the file download it from the server.
	 */
	void PlaySound(const FString& Sound, float StartPoint);

	void StopSound();

	UFUNCTION()
	void OnRep_Playback();

	/**
	 * Called on the server when the sound reached its end.
	 * Triggers a network signal notifyng that the audio has stoped playing.
	 * Also resets CurrentSound
	 */
	void OnSoundFinished();

	/**
	 * Returns meta-data for this type in the FINReflection-System
	 */
	UFUNCTION()
	void netClass_Meta(FString& InternalName, FText& DisplayName, FText& Description);

	/**
	 * Loads and Plays the sound file in the Sounds folder appened with the given relative path without the file extension.
	 * Plays the sound at the given startPoint.
	 * Might cause the current sound playing to stop even if the new sound is not found.
	 * If able to play the sound, emits a play sound signal.
	 */
	UFUNCTION()
	void netFunc_playSound(const FString& sound, float startPoint);
	UFUNCTION()
	void netFuncMeta_playSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);

	/**
	 * Stops the current playing sound.
	 * Emits a stop sound signal if it actually was able to stop the current playing sound.
	 */
	UFUNCTION()
	void netFunc_stopSound();
	UFUNCTION()
    void netFuncMeta_stopSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);

	/**
	 * Notifies when the state of the speaker pole has changed.
	 * f.e. if the sound stoped/started playing
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void netSig_SpeakerSound(int type, const FString& sound);
	UFUNCTION()
    void netSigMeta_SpeakerSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);


private:
	/** Plays the replicated playback locally, or stops the local sound if nothing is playing */
	void PlayLocally();

	/** Starts the decoded sound at the current position of the server's playback */
	void StartDecodedSound(const TArray<uint8>& PCM, int32 SampleRate, int32 NumChannels);

	UPROPERTY(ReplicatedUsing=OnRep_Playback)
	FFINSpeakerPlayback Playback;

	/** Keeps the sound file in the media cache while the sound plays, so clients can still download it */
	TSharedPtr<const TArray<uint8>> PlaybackData;

	/** Server only, fires when the sound reached its end */
	FTimerHandle FinishedTimer;

	/** Stops the local sound at its end, procedural sound waves don't finish on their own */
	FTimerHandle LocalStopTimer;

	/** Increased with every local play/stop, so outdated downloads and decodes get dropped */
	uint32 PlayRequest = 0;
};
