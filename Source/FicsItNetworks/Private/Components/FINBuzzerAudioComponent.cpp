#include "Components/FINBuzzerAudioComponent.h"

#include "Components/FINSpeakerPole.h"
#include "Async/Async.h"

UFINBuzzerAudioComponent::UFINBuzzerAudioComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	SetIsReplicatedByDefault(true);
}

void UFINBuzzerAudioComponent::BeginPlay() {
	Super::BeginPlay();

	EnsureEventLoaded();
	ApplyAudioSettings();
}

void UFINBuzzerAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	StopBeepLocally();
	Super::EndPlay(EndPlayReason);
}

void UFINBuzzerAudioComponent::ApplyAudioSettings() {
	// Same Wwise event as the speaker pole: its attenuation fades to silence at the speaker's base range
	SetAttenuationScalingFactor(FMath::Max(Range, 0.01f) / AFINSpeakerPole::BaseAttenuationRange);
}

void UFINBuzzerAudioComponent::Beep(float Frequency, float Volume, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve) {
	// The blueprint may call this on clients too, with values the server never replicated (buildings are net dormant)
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	GetOwner()->FlushNetDormancy();
	Multicast_Beep(Frequency, Volume, AttackTime, AttackCurve, DecayTime, DecayCurve);
}

void UFINBuzzerAudioComponent::StopBeep() {
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	GetOwner()->FlushNetDormancy();
	Multicast_StopBeep();
}

void UFINBuzzerAudioComponent::Multicast_Beep_Implementation(float Frequency, float Volume, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve) {
	// No audio on a dedicated server
	if (GetNetMode() == NM_DedicatedServer) return;
	PlayBeepLocally(Frequency, Volume, AttackTime, AttackCurve, DecayTime, DecayCurve);
}

void UFINBuzzerAudioComponent::Multicast_StopBeep_Implementation() {
	StopBeepLocally();
}

void UFINBuzzerAudioComponent::PlayBeepLocally(float Frequency, float Volume, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve) {
	bool bNeedsPost;
	{
		FScopeLock ScopeLock(&Lock);
		BeepFrequency = FMath::Clamp(Frequency, 0.f, SampleRate / 2.f);
		BeepVolume = FMath::Clamp(Volume, 0.f, 1.f);
		BeepAttackTime = FMath::Max(AttackTime, 0.f);
		BeepAttackCurve = FMath::Max(AttackCurve, 0.01f);
		BeepDecayTime = FMath::Max(DecayTime, 0.f);
		BeepDecayCurve = FMath::Max(DecayCurve, 0.01f);
		BeepPosition = 0;
		// A running voice just picks up the restarted envelope
		bNeedsPost = !bVoiceActive;
		bVoiceActive = true;
	}
	if (bNeedsPost) {
		// Make sure no voice of a previous beep is left, every voice pulls samples from this generator
		Stop();
		PostInputEvent();
	}
}

void UFINBuzzerAudioComponent::StopBeepLocally() {
	CancelPostInputEvent();
	{
		FScopeLock ScopeLock(&Lock);
		bVoiceActive = false;
	}
	Stop();
}

bool UFINBuzzerAudioComponent::IsBeeping() {
	FScopeLock ScopeLock(&Lock);
	return bVoiceActive;
}

bool UFINBuzzerAudioComponent::FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) {
	// Called on the Wwise audio thread
	FScopeLock ScopeLock(&Lock);

	const int64 AttackSamples = static_cast<int64>(BeepAttackTime * SampleRate);
	const int64 DecaySamples = static_cast<int64>(BeepDecayTime * SampleRate);
	const int64 TotalSamples = AttackSamples + DecaySamples;
	if (!bVoiceActive || BeepPosition >= TotalSamples) {
		for (uint32 Channel = 0; Channel < NumChannels; ++Channel) {
			FMemory::Memzero(BufferToFill[Channel], NumSamples * sizeof(float));
		}
		if (bVoiceActive) {
			bVoiceActive = false;
			// Returning false only reports "no data ready", the Wwise voice keeps running until it gets stopped
			AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<UFINBuzzerAudioComponent>(this)]() {
				UFINBuzzerAudioComponent* This = WeakThis.Get();
				if (This && !This->IsBeeping()) {
					This->Stop();
				}
			});
		}
		return false;
	}

	const double PhaseIncrement = 2.0 * PI * BeepFrequency / SampleRate;
	for (uint32 Sample = 0; Sample < NumSamples; ++Sample) {
		float Envelope = 0.f;
		if (BeepPosition < AttackSamples) {
			Envelope = FMath::Pow(static_cast<float>(BeepPosition) / AttackSamples, BeepAttackCurve);
		} else if (BeepPosition < TotalSamples) {
			const float DecayProgress = static_cast<float>(BeepPosition - AttackSamples) / DecaySamples;
			Envelope = FMath::Pow(1.f - DecayProgress, BeepDecayCurve);
		}
		const float Value = Envelope * BeepVolume * static_cast<float>(FMath::Sin(Phase));
		for (uint32 Channel = 0; Channel < NumChannels; ++Channel) {
			BufferToFill[Channel][Sample] = Value;
		}
		Phase = FMath::Fmod(Phase + PhaseIncrement, 2.0 * PI);
		++BeepPosition;
	}
	// The remaining samples of the last buffer are silent, the voice stops with the next call
	return true;
}

void UFINBuzzerAudioComponent::GetChannelConfig(AkAudioFormat& AudioFormat) {
	AkChannelConfig ChannelConfig(1, AK::ChannelMaskFromNumChannels(1));
	// Float samples have to be non-interleaved, block align is per channel then (same as Wwise's AudioLink input client)
	AudioFormat.SetAll(SampleRate, ChannelConfig, 32, sizeof(float), AK_FLOAT, AK_NONINTERLEAVED);
}
