#pragma once

#include "Components/AudioComponent.h"
#include "AudioController.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINKernelAudioController : public UActorComponent {
	GENERATED_BODY()
	
private:
	UPROPERTY()
	UAudioComponent* AudioComponent = nullptr;
	
	UFUNCTION(NetMulticast, Unreliable)
	void ExecBeep(float InPitch);
	
public:
	UFINKernelAudioController();
	
	// Begin UActorComponent
	virtual bool IsSupportedForNetworking() const override;
	// End UActorComponent
	
	/**
	 * Sets the audio component this controller is associated with
	 *
	 * @param[in]	InSpeaker	the audio component of this controller
	 */
	void SetComponent(UAudioComponent* InSpeaker);

	/**
	 * Plays a short beep sound
	 */
	virtual void Beep(float InBeep = 1.0f);
};
