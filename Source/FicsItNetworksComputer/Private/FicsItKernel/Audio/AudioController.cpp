#include "FicsItKernel/Audio/AudioController.h"

#include "AkAudioEvent.h"
#include "AkComponent.h"

void UFINKernelAudioController::ExecBeep_Implementation(float Pitch) {
	if (IsValid(Speaker) && IsValid(BeepEvent)) {
		// pitch multiplier to cents, Wwise allows +-2 octaves
		const float Cents = FMath::Clamp(1200.f * FMath::Log2(FMath::Max(Pitch, 0.25f)), -2400.f, 2400.f);
		Speaker->SetRTPCValue(nullptr, Cents, 0, TEXT("FIN_Beep_Pitch"));
		Speaker->PostAkEvent(BeepEvent, 0, FOnAkPostEventCallback());
	}
}

UFINKernelAudioController::UFINKernelAudioController() {

}

bool UFINKernelAudioController::IsSupportedForNetworking() const {
	return true;
}

void UFINKernelAudioController::SetComponent(UAkComponent* InSpeaker, UAkAudioEvent* InBeepEvent) {
	Speaker = InSpeaker;
	BeepEvent = InBeepEvent;
}

void UFINKernelAudioController::Beep(float InBeep) {
	ExecBeep(InBeep);
}
