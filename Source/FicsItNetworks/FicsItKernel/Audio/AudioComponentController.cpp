#include "AudioComponentController.h"

namespace FicsItKernel {
	namespace Audio {
		AudioComponentController::AudioComponentController(UFINAudioComponentControllerTrampoline* Trampoline) {
			this->Trampoline = Trampoline;
		}

		void AudioComponentController::beep() {
			Trampoline->beep();
		}
	}
}

UFINAudioComponentControllerTrampoline::UFINAudioComponentControllerTrampoline() {
	bReplicates = true;
}

bool UFINAudioComponentControllerTrampoline::IsSupportedForNetworking() const {
	return true;
}

void UFINAudioComponentControllerTrampoline::beep_Implementation() {
	if (IsValid(Speaker)) {
		Speaker->Play();
	}
}
