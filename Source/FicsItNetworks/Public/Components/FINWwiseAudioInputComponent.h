#pragma once

#include "CoreMinimal.h"
#include "AkAudioInputComponent.h"
#include "FINWwiseAudioInputComponent.generated.h"

/**
 * Base for components that feed generated audio samples into Wwise through the "Wwise Audio Input" source plugin.
 * Satisfactory disables the Unreal audio engine, so this is the way to play runtime generated audio.
 * The AkAudioEvent of the component has to be set to Play_FIN_Speaker (or another event using the Audio Input source).
 */
UCLASS(Abstract)
class FICSITNETWORKS_API UFINWwiseAudioInputComponent : public UAkAudioInputComponent {
	GENERATED_BODY()
public:
	UFINWwiseAudioInputComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Makes sure the Wwise event and its SoundBank are loaded.
	 * The event gets loaded together with the owning blueprint while the mod initializes. At that time Wwise might not have
	 * loaded its Init bank yet, so the SoundBank fails to load and Wwise doesn't retry on its own.
	 */
	void EnsureEventLoaded();

protected:
	/** Posts the audio input event, retries for a moment while the event data is still loading. */
	void PostInputEvent();

	/** Cancels a pending post of the audio input event. */
	void CancelPostInputEvent();

private:
	void PostWhenLoaded(uint32 ForRequest, int32 RetriesLeft);

	FTimerHandle PostRetryTimer;
	uint32 PostRequest = 0;
};
