#pragma once

#include "CoreMinimal.h"
#include "FINWwiseAudioInputComponent.h"
#include "FINBuzzerAudioComponent.generated.h"

/**
 * Synthesizes the buzzer tone (sine oscillator with an attack/decay envelope) and plays it through Wwise.
 * Replaces the MetaSound based buzzer, MetaSounds need the Unreal audio engine which is disabled in Satisfactory.
 */
UCLASS(ClassGroup=FicsItNetworks, meta=(BlueprintSpawnableComponent))
class FICSITNETWORKS_API UFINBuzzerAudioComponent : public UFINWwiseAudioInputComponent {
	GENERATED_BODY()
public:
	UFINBuzzerAudioComponent(const FObjectInitializer& ObjectInitializer);

	/** Range in meters at which the buzzer fades out to silence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buzzer")
	float Range = 30.f;

	// Begin UActorComponent
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End UActorComponent

	/**
	 * Plays one beep on the server and all clients. Restarts the envelope if a beep is still playing.
	 * Only has an effect on the server, the parameters get sent to the clients with the call.
	 * @param Frequency		frequency of the sine tone in Hz
	 * @param Volume		volume multiplier of the tone (0 to 1)
	 * @param AttackTime	time in seconds to rise to full volume
	 * @param AttackCurve	curve of the attack, 1 = linear, < 1 rises fast, > 1 rises slow
	 * @param DecayTime		time in seconds to fade out after the attack
	 * @param DecayCurve	curve of the decay, 1 = linear, < 1 falls slow, > 1 falls fast
	 */
	UFUNCTION(BlueprintCallable, Category="Buzzer")
	void Beep(float Frequency, float Volume, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve);

	/** Stops the current beep on the server and all clients. Only has an effect on the server. */
	UFUNCTION(BlueprintCallable, Category="Buzzer")
	void StopBeep();

	/** True while a beep is playing. */
	UFUNCTION(BlueprintPure, Category="Buzzer")
	bool IsBeeping();

protected:
	// Begin UAkAudioInputComponent
	virtual bool FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) override;
	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;
	// End UAkAudioInputComponent

private:
	void ApplyAudioSettings();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Beep(float Frequency, float Volume, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopBeep();

	void PlayBeepLocally(float Frequency, float Volume, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve);
	void StopBeepLocally();

	static constexpr uint32 SampleRate = 48000;

	FCriticalSection Lock;
	/** Parameters of the current beep, accessed by the audio thread */
	float BeepFrequency = 0.f;
	float BeepVolume = 0.f;
	float BeepAttackTime = 0.f;
	float BeepAttackCurve = 1.f;
	float BeepDecayTime = 0.f;
	float BeepDecayCurve = 1.f;
	/** Samples since the beep started */
	int64 BeepPosition = 0;
	double Phase = 0.0;
	/** True while a beep is playing. Once the envelope ended the voice gets stopped on the game thread, returning false from FillSamplesBuffer alone keeps it running */
	bool bVoiceActive = false;

	FDelegateHandle GlobalVolumeHandle;
};
