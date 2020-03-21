#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "Components/AudioComponent.h"
#include "Network/FINNetworkConnector.h"
#include "FINSpeakerPole.generated.h"

UCLASS(Blueprintable)
class AFINSpeakerPole : public AFGBuildable {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
		UFINNetworkConnector* NetworkConnector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SpeakerPole")
		UAudioComponent* AudioComponent;

	UPROPERTY(BlueprintReadOnly, Category="SpeakerPole")
		FString CurrentSound;

	AFINSpeakerPole();

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
		void netFunc_PlaySound(const FString& sound, float startPoint);

	/**
	 * Stops the current playing sound.
	 * Emits a stop sound signal if it actually was able to stop the current playing sound.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Component")
		void netFunc_StopSound();

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