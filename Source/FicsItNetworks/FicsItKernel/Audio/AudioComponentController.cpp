#include "AudioComponentController.h"

namespace FicsItKernel {
	namespace Audio {
		AudioComponentController::AudioComponentController(UFINAudioComponentControllerTrampoline* Trampoline) {
			this->Trampoline = Trampoline;
		}

		void AudioComponentController::beep(float Pitch) {
			Trampoline->beep(Pitch);
		}
	}
}

UFINAudioComponentControllerTrampoline::UFINAudioComponentControllerTrampoline() {
	bReplicates = true;
}

bool UFINAudioComponentControllerTrampoline::IsSupportedForNetworking() const {
	return true;
}

void UFINAudioComponentControllerTrampoline::beep_Implementation(float Pitch) {
	if (IsValid(Speaker)) {
		Speaker->SetPitchMultiplier(Pitch);
		Speaker->Play();
	}
}
