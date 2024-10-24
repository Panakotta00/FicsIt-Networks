#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioController.generated.h"

class UAudioComponent;

UCLASS()
class FICSITNETWORKSCOMPUTER_API UFINKernelAudioController : public UActorComponent {
	GENERATED_BODY()
	
private:
	UPROPERTY()
	UAudioComponent* AudioComponent = nullptr;
	
	UFUNCTION(NetMulticast, Reliable)
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
