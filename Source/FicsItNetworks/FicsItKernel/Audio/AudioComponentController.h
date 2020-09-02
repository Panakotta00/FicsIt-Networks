#pragma once

#include "CoreMinimal.h"
#include "AudioController.h"

#include "AudioComponentController.generated.h"

UCLASS()
class UFINAudioComponentControllerTrampoline : public UActorComponent {
	GENERATED_BODY()
public:
	UAudioComponent* Speaker = nullptr;

	UFINAudioComponentControllerTrampoline();

	// Begin UActorComponent
	virtual bool IsSupportedForNetworking() const override;
	// End UActorComponent
	
	UFUNCTION(NetMulticast, Reliable)
	void beep();
};

namespace FicsItKernel {
	namespace Audio {
		class AudioComponentController : public AudioController {
		public:
            /**
            * The underlying audio component used to play the audio
            */
            UFINAudioComponentControllerTrampoline* Trampoline;

			AudioComponentController(UFINAudioComponentControllerTrampoline* Trampoline);

			// Begin AudioController
			virtual void beep() override;
			// End AudioController
		};
	}
}