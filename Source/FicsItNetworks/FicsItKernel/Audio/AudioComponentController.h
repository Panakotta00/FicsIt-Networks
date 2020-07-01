#pragma once

#include "AudioController.h"

namespace FicsItKernel {
	namespace Audio {
		class AudioComponentController : public AudioController {
		public:
            /**
            * The underlying audio component used to play the audio
            */
            UAudioComponent* Speaker;

			AudioComponentController(UAudioComponent* Speaker);

			// Begin AudioController
			virtual void beep() override;
			// End AudioController
		};
	}
}