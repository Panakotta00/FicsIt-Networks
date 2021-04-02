#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
#include "FINSpeakerPole.generated.h"

UCLASS(Blueprintable)
class AFINSpeakerPole : public AFGBuildable, public IFINSignalSender {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
	UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
	UAudioComponent* AudioComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="SpeakerPole")
	FString CurrentSound;

	AFINSpeakerPole();

	// Begin IFINNetworkSignalSender
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINNetworkSignalSender

	UFUNCTION(NetMulticast, Reliable)
	void PlaySound(const FString& Sound, float StartPoint);

	UFUNCTION(NetMulticast, Reliable)
	void StopSound();
	
	/**
	 * Event bound to the OnAudioFinished event of the AudioComponent.
	 * Triggers a network signal notifyng that the audio has stoped playing.
	 * Also resets CurrentSound
	 */
	UFUNCTION()
	void OnSoundFinished(UAudioComponent* Audio);

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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_SpeakerSound(int type, const FString& sound);
	UFUNCTION()
    void netSigMeta_SpeakerSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime);


	/**
	 * Loads the sound file referenced by the given relative path
	 * from the %localappdata%/FactoryGame/Saved/SaveGames/Computers/Sounds folder
	 * without the file extension.
	 * If it was unable to load the sound file, return nullptr.
	 */
	USoundWave* LoadSoundFromFile(const FString& sound);
};
