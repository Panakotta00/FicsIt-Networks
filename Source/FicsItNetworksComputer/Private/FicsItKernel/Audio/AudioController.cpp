#include "FicsItKernel/Audio/AudioController.h"

void UFINKernelAudioController::ExecBeep_Implementation(float Pitch) {
	if (IsValid(AudioComponent)) {
		AudioComponent->SetPitchMultiplier(Pitch);
		AudioComponent->Play();
	}
}

UFINKernelAudioController::UFINKernelAudioController() {
	
}

bool UFINKernelAudioController::IsSupportedForNetworking() const {
	return true;
}

void UFINKernelAudioController::SetComponent(UAudioComponent* InSpeaker) {
	AudioComponent = InSpeaker;
}

void UFINKernelAudioController::Beep(float InBeep) {
	ExecBeep(InBeep);
}
