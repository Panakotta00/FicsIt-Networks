#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioController.generated.h"

class UAkComponent;
class UAkAudioEvent;

UCLASS()
class FICSITNETWORKSCOMPUTER_API UFINKernelAudioController : public UActorComponent {
	GENERATED_BODY()

private:
	UPROPERTY()
	UAkComponent* Speaker = nullptr;

	UPROPERTY()
	UAkAudioEvent* BeepEvent = nullptr;

	UFUNCTION(NetMulticast, Reliable)
	void ExecBeep(float InPitch);

public:
	UFINKernelAudioController();

	// Begin UActorComponent
	virtual bool IsSupportedForNetworking() const override;
	// End UActorComponent

	/**
	 * Sets the Wwise component and event this controller plays the beep with
	 *
	 * @param[in]	InSpeaker	the Wwise component the beep is played on
	 * @param[in]	InBeepEvent	the Wwise event of the beep, its pitch is controlled by the RTPC FIN_Beep_Pitch (in cents)
	 */
	void SetComponent(UAkComponent* InSpeaker, UAkAudioEvent* InBeepEvent);

	/**
	 * Plays a short beep sound
	 */
	virtual void Beep(float InBeep = 1.0f);
};
