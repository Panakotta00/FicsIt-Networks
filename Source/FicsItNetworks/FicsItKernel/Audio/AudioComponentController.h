#pragma once

#include "CoreMinimal.h"
#include "AudioController.h"

#include "AudioComponentController.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINAudioComponentControllerTrampoline : public UActorComponent {
	GENERATED_BODY()
public:
	UAudioComponent* Speaker = nullptr;

	UFINAudioComponentControllerTrampoline();

	// Begin UActorComponent
	virtual bool IsSupportedForNetworking() const override;
	// End UActorComponent
	
	UFUNCTION(NetMulticast, Reliable)
	void beep(float Pitch);
};

namespace FicsItKernel {
	namespace Audio {
		class FICSITNETWORKS_API AudioComponentController : public AudioController {
		public:
            /**
            * The underlying audio component used to play the audio
            */
            UFINAudioComponentControllerTrampoline* Trampoline;

			AudioComponentController(UFINAudioComponentControllerTrampoline* Trampoline);

			// Begin AudioController
			virtual void beep(float Pitch) override;
			// End AudioController
		};
	}
}