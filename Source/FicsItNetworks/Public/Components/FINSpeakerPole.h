#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "Signals/FINSignalSender.h"
#include "FINSpeakerPole.generated.h"

struct FFINSpeakerSoundData;

UCLASS(Blueprintable)
class AFINSpeakerPole : public AFGBuildable, public IFINSignalSender {
	GENERATED_BODY()

public:
	/** Distance in meters at which the base attenuation of the speaker sound reaches silence. Range gets scaled relative to this. */
	static constexpr float BaseAttenuationRange = 100.f;
	static constexpr float MaxVolume = 4.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
	class UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	/** Plays the sound through Wwise. Its AkAudioEvent has to be set to Play_FIN_Speaker. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SpeakerPole")
	class UFINSpeakerAudioComponent* SpeakerAudio = nullptr;

	/** Deprecated: Unreal audio is disabled in Satisfactory, use SpeakerAudio. Only kept until the blueprint no longer references it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
	class UAudioComponent* AudioComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="SpeakerPole")
	FString CurrentSound;

	/** Range in meters at which the sound fades out to silence. */
	UPROPERTY(SaveGame, ReplicatedUsing=OnRep_AudioSettings, BlueprintReadOnly, Category="SpeakerPole")
	float Range = 30.f;

	/** Volume multiplier of this speaker, 1 = original volume, up to 4 = amplified. */
	UPROPERTY(SaveGame, ReplicatedUsing=OnRep_AudioSettings, BlueprintReadOnly, Category="SpeakerPole")
	float Volume = 1.f;

	AFINSpeakerPole();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End AActor

	// Begin IFINNetworkSignalSender
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINNetworkSignalSender

	UFUNCTION(NetMulticast, Reliable)
	void PlaySound(const FString& Sound, float StartPoint);

	UFUNCTION(NetMulticast, Reliable)
	void StopSound();

	UFUNCTION()
	void OnRep_AudioSettings();

	/**
	 * Called when the sound reached its end.
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

	UFUNCTION()
	void netFunc_setRange(float InRange);
	UFUNCTION()
	void netFuncMeta_setRange(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);

	UFUNCTION()
	float netFunc_getRange();
	UFUNCTION()
	void netFuncMeta_getRange(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);

	UFUNCTION()
	void netFunc_setVolume(float InVolume);
	UFUNCTION()
	void netFuncMeta_setVolume(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);

	UFUNCTION()
	float netFunc_getVolume();
	UFUNCTION()
	void netFuncMeta_getVolume(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);

	/**
	 * Notifies when the state of the speaker pole has changed.
	 * f.e. if the sound stoped/started playing
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void netSig_SpeakerSound(int type, const FString& sound);
	UFUNCTION()
    void netSigMeta_SpeakerSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);

	/**
	 * Notifies when a setting of the speaker pole has changed.
	 * Setting 0 = range, 1 = volume.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void netSig_SpeakerSetting(int Setting, float New, float Old);
	UFUNCTION()
	void netSigMeta_SpeakerSetting(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);

	/**
	 * Reads the sound file referenced by the given relative path
	 * from the %localappdata%/FactoryGame/Saved/SaveGames/Computers/Sounds folder
	 * without the file extension.
	 * Returns false if the file is outside of the sounds folder or could not be read.
	 */
	static bool LoadSoundFile(const FString& InSound, TArray<uint8>& OutData);

private:
	void ApplyAudioSettings();

	/** Incremented for every play/stop request, so a decoded sound of an outdated request gets dropped. */
	uint32 PlayRequest = 0;
};
