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
			virtual void beep() = 0;
		};
	}
}