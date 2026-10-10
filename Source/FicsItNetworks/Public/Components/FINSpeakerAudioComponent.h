#pragma once

#include "CoreMinimal.h"
#include "FINWwiseAudioInputComponent.h"
#include "FINSpeakerAudioComponent.generated.h"

/**
 * Decoded PCM data of a sound file, shared between the game thread and the Wwise audio thread.
 */
struct FFINSpeakerSoundData {
	int32 SampleRate = 0;
	int32 NumChannels = 0;
	/** Interleaved 16-bit samples */
	TArray<int16> Samples;

	int64 GetNumFrames() const { return NumChannels > 0 ? Samples.Num() / NumChannels : 0; }
};

DECLARE_MULTICAST_DELEGATE(FFINSpeakerAudioFinished);

/**
 * Plays decoded sound data through Wwise using the "Wwise Audio Input" source plugin.
 * This way the speaker uses the game's audio pipeline (volume sliders, output device switching, spatialization).
 */
UCLASS()
class FICSITNETWORKS_API UFINSpeakerAudioComponent : public UFINWwiseAudioInputComponent {
	GENERATED_BODY()
public:
	UFINSpeakerAudioComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Decodes the given OGG Vorbis file content.
	 * Can be called from any thread.
	 */
	static TSharedPtr<const FFINSpeakerSoundData> DecodeOgg(const TArray<uint8>& FileData);

	/** Starts playing the given data at the given start point in seconds. Stops the currently playing sound. */
	void PlayData(const TSharedRef<const FFINSpeakerSoundData>& InData, float StartSeconds);

	/** Stops the currently playing sound without broadcasting OnFinished. */
	void StopData();

	bool IsPlayingData() const;

	/** Broadcast on the game thread when the sound reached its end. */
	FFINSpeakerAudioFinished OnFinished;

protected:
	// Begin UAkAudioInputComponent
	virtual bool FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) override;
	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;
	// End UAkAudioInputComponent

private:
	mutable FCriticalSection DataLock;
	TSharedPtr<const FFINSpeakerSoundData> Data;
	int64 FramePosition = 0;
	/** Incremented on every play/stop, so a finished notification of an old sound gets ignored. */
	uint32 Generation = 0;
	bool bFinishedNotified = false;
};
