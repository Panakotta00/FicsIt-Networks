#include "Components/FINSpeakerAudioComponent.h"

#include "Async/Async.h"
#include "AudioDecompress.h"
#include "Decoders/VorbisAudioInfo.h"

UFINSpeakerAudioComponent::UFINSpeakerAudioComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

TSharedPtr<const FFINSpeakerSoundData> UFINSpeakerAudioComponent::DecodeOgg(const TArray<uint8>& FileData) {
	FSoundQualityInfo Quality;
	FVorbisAudioInfo Vorbis;
	if (!Vorbis.ReadCompressedInfo(FileData.GetData(), FileData.Num(), &Quality)) return nullptr;
	if (Quality.NumChannels == 0 || Quality.SampleRate == 0) return nullptr;

	TSharedRef<FFINSpeakerSoundData> Result = MakeShared<FFINSpeakerSoundData>();
	Result->SampleRate = Quality.SampleRate;
	Result->NumChannels = Quality.NumChannels;
	Result->Samples.SetNumUninitialized(Quality.SampleDataSize / sizeof(int16));
	Vorbis.ExpandFile(reinterpret_cast<uint8*>(Result->Samples.GetData()), &Quality);
	return Result;
}

void UFINSpeakerAudioComponent::PlayData(const TSharedRef<const FFINSpeakerSoundData>& InData, float StartSeconds) {
	StopData();
	{
		FScopeLock Lock(&DataLock);
		Data = InData;
		FramePosition = FMath::Clamp<int64>(static_cast<int64>(FMath::Max(0.f, StartSeconds) * InData->SampleRate), 0, InData->GetNumFrames());
		bFinishedNotified = false;
		++Generation;
	}
	PostInputEvent();
}

void UFINSpeakerAudioComponent::StopData() {
	CancelPostInputEvent();
	{
		FScopeLock Lock(&DataLock);
		Data.Reset();
		FramePosition = 0;
		++Generation;
	}
	Stop();
}

bool UFINSpeakerAudioComponent::IsPlayingData() const {
	FScopeLock Lock(&DataLock);
	return Data.IsValid() && FramePosition < Data->GetNumFrames();
}

bool UFINSpeakerAudioComponent::FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) {
	// Called on the Wwise audio thread
	FScopeLock Lock(&DataLock);

	const int64 NumFrames = Data.IsValid() ? Data->GetNumFrames() : 0;
	if (!Data.IsValid() || FramePosition >= NumFrames) {
		for (uint32 Channel = 0; Channel < NumChannels; ++Channel) {
			FMemory::Memzero(BufferToFill[Channel], NumSamples * sizeof(float));
		}
		if (Data.IsValid() && !bFinishedNotified) {
			bFinishedNotified = true;
			AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<UFINSpeakerAudioComponent>(this), FinishedGeneration = Generation]() {
				UFINSpeakerAudioComponent* This = WeakThis.Get();
				if (This && This->Generation == FinishedGeneration) {
					// Returning false only reports "no data ready", the Wwise voice keeps running until it gets stopped
					This->Stop();
					This->OnFinished.Broadcast();
				}
			});
		}
		return false;
	}

	const int32 SourceChannels = Data->NumChannels;
	const int16* Samples = Data->Samples.GetData();
	for (uint32 Frame = 0; Frame < NumSamples; ++Frame) {
		const int64 SourceFrame = FramePosition + Frame;
		for (uint32 Channel = 0; Channel < NumChannels; ++Channel) {
			float Value = 0.f;
			if (SourceFrame < NumFrames) {
				const int32 SourceChannel = FMath::Min<int32>(Channel, SourceChannels - 1);
				Value = Samples[SourceFrame * SourceChannels + SourceChannel] / 32768.f;
			}
			BufferToFill[Channel][Frame] = Value;
		}
	}
	FramePosition = FMath::Min(FramePosition + NumSamples, NumFrames);
	return true;
}

void UFINSpeakerAudioComponent::GetChannelConfig(AkAudioFormat& AudioFormat) {
	FScopeLock Lock(&DataLock);
	const uint32 SampleRate = Data.IsValid() ? Data->SampleRate : 48000;
	// Mono stays mono, everything else is played as stereo (additional channels are dropped)
	const uint32 NumChannels = (Data.IsValid() && Data->NumChannels == 1) ? 1 : 2;
	AkChannelConfig ChannelConfig(NumChannels, AK::ChannelMaskFromNumChannels(NumChannels));
	// Float samples have to be non-interleaved, block align is per channel then (same as Wwise's AudioLink input client)
	AudioFormat.SetAll(SampleRate, ChannelConfig, 32, sizeof(float), AK_FLOAT, AK_NONINTERLEAVED);
}
