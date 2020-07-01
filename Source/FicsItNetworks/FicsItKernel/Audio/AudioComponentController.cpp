#include "AudioComponentController.h"

namespace FicsItKernel {
	namespace Audio {
		AudioComponentController::AudioComponentController(UAudioComponent* Speaker) {
			this->Speaker = Speaker;
		}

		void AudioComponentController::beep() {
			if (IsValid(Speaker)) {
				Speaker->Play();
			}
		}
	}
}
