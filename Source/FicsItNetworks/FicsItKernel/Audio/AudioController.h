#pragma once
#include "Components/AudioComponent.h"

namespace FicsItKernel {
	namespace Audio {
		class AudioController {
		public:
			virtual ~AudioController() = default;
			
			/**
			 * Plays a short beep sound
			 */
			virtual void beep(float Beep = 1.0f) = 0;
		};
	}
}