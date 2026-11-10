#include "Components/FINSpeakerPole.h"
#include "FicsItNetworksModule.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINMediaSubsystem.h"
#include "Async/Async.h"
#include "Components/AudioComponent.h"
#include "Decoders/VorbisAudioInfo.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/IAudioFormat.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundWaveProcedural.h"
#include "TimerManager.h"

namespace {
	double GetServerWorldTime(const UWorld* World) {
		const AGameStateBase* GameState = World->GetGameState();
		return GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
	}

	/** Decoded 16 bit PCM data of a sound file */
	struct FDecodedSound {
		TArray<uint8> PCM;
		int32 SampleRate = 0;
		int32 NumChannels = 0;
	};

	TSharedPtr<FDecodedSound> DecodeOgg(const TArray<uint8>& FileData) {
		FSoundQualityInfo Quality;
		FVorbisAudioInfo Vorbis;
		if (!Vorbis.ReadCompressedInfo(FileData.GetData(), FileData.Num(), &Quality) || Quality.NumChannels == 0 || Quality.SampleRate == 0) return nullptr;

		TSharedPtr<FDecodedSound> Sound = MakeShared<FDecodedSound>();
		Sound->SampleRate = Quality.SampleRate;
		Sound->NumChannels = Quality.NumChannels;
		Sound->PCM.AddUninitialized(Quality.SampleDataSize);
		Vorbis.ExpandFile(Sound->PCM.GetData(), &Quality);
		return Sound;
	}
}

AFINSpeakerPole::AFINSpeakerPole() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->SetIsReplicated(true);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	AudioComponent->SetupAttachment(RootComponent);
}

void AFINSpeakerPole::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	GetWorldTimerManager().ClearTimer(FinishedTimer);
	GetWorldTimerManager().ClearTimer(LocalStopTimer);
	++PlayRequest;
	PlaybackData.Reset();
	Super::EndPlay(EndPlayReason);
}

void AFINSpeakerPole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFINSpeakerPole, Playback);
}

UObject* AFINSpeakerPole::GetSignalSenderOverride_Implementation() {
	return NetworkConnector;
}

void AFINSpeakerPole::PlaySound(const FString& Sound, float StartPoint) {
	if (!HasAuthority()) return;
	AFINMediaSubsystem* Media = AFINMediaSubsystem::GetMediaSubsystem(this);
	if (!Media) return;

	TArray<uint8> FileData;
	if (!AFINMediaSubsystem::LoadSoundFile(Sound, FileData)) return;
	if (FileData.Num() > AFINMediaSubsystem::MaxSoundFileSize) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Sound file '%s' is too large (%d bytes, max %d bytes)."), *Sound, FileData.Num(), AFINMediaSubsystem::MaxSoundFileSize);
		return;
	}

	// Only reads the header, a dedicated server doesn't decode the sound.
	// Without audio (dedicated server) nothing starts the Vorbis decoder module, which loads the Vorbis DLLs.
	LoadVorbisLibraries();
	FSoundQualityInfo Quality;
	FVorbisAudioInfo Vorbis;
	if (!Vorbis.ReadCompressedInfo(FileData.GetData(), FileData.Num(), &Quality) || Quality.NumChannels == 0 || Quality.SampleRate == 0) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Sound file '%s' is not a valid OGG Vorbis file."), *Sound);
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(FinishedTimer)) {
		netSig_SpeakerSound(1, CurrentSound);
	}

	const int32 Size = FileData.Num();
	const FString Hash = Media->AddSound(MoveTemp(FileData));
	// Keeps the file in the cache while clients may still download it
	PlaybackData = Media->FindSound(Hash);

	Playback.Sound = Sound;
	Playback.Hash = Hash;
	Playback.Size = Size;
	Playback.StartPoint = FMath::Max(0.f, StartPoint);
	Playback.Duration = Quality.Duration;
	Playback.ServerStartTime = GetServerWorldTime(GetWorld());
	// Buildings are net dormant, the playback wouldn't reach the clients otherwise
	FlushNetDormancy();
	ForceNetUpdate();

	CurrentSound = Sound;
	netSig_SpeakerSound(0, CurrentSound);
	PlayLocally();

	// The server decides when the sound finished, a dedicated server doesn't play it
	const float Remaining = Playback.Duration - Playback.StartPoint;
	if (Remaining > 0.f) {
		GetWorldTimerManager().SetTimer(FinishedTimer, this, &AFINSpeakerPole::OnSoundFinished, Remaining, false);
	} else {
		GetWorldTimerManager().ClearTimer(FinishedTimer);
		OnSoundFinished();
	}
}

void AFINSpeakerPole::StopSound() {
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(FinishedTimer);
	Playback = FFINSpeakerPlayback();
	PlaybackData.Reset();
	FlushNetDormancy();
	ForceNetUpdate();
	PlayLocally();

	netSig_SpeakerSound(1, CurrentSound);
	CurrentSound = "";
}

void AFINSpeakerPole::OnRep_Playback() {
	PlayLocally();
}

void AFINSpeakerPole::OnSoundFinished() {
	// The playback stays replicated, so clients finish the sound themselves instead of getting cut off because of the latency.
	// Clients joining later don't play it, it lies in the past.
	PlaybackData.Reset();
	netSig_SpeakerSound(2, CurrentSound);
	CurrentSound = "";
}

void AFINSpeakerPole::PlayLocally() {
	const uint32 Request = ++PlayRequest;

	// Stop the previous sound while the new one gets downloaded and decoded
	GetWorldTimerManager().ClearTimer(LocalStopTimer);
	AudioComponent->Stop();

	// No audio on a dedicated server
	if (!Playback.IsSet() || GetNetMode() == NM_DedicatedServer) return;

	const double Elapsed = GetServerWorldTime(GetWorld()) - Playback.ServerStartTime;
	if (Playback.StartPoint + Elapsed >= Playback.Duration) return;

	AFINMediaSubsystem* Media = AFINMediaSubsystem::GetMediaSubsystem(this);
	if (!Media) return;

	Media->RequestSound(Playback.Sound, Playback.Hash, Playback.Size, FFINMediaSoundLoaded::CreateWeakLambda(this, [this, Request](TSharedPtr<const TArray<uint8>> FileData) {
		if (PlayRequest != Request || !FileData.IsValid()) return;

		// Decoding a whole file takes a moment, don't block the game thread
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis = TWeakObjectPtr<AFINSpeakerPole>(this), FileData = FileData.ToSharedRef(), Sound = Playback.Sound, Request]() {
			TSharedPtr<FDecodedSound> Decoded = DecodeOgg(*FileData);
			if (!Decoded.IsValid()) {
				UE_LOG(LogFicsItNetworks, Warning, TEXT("Sound file '%s' is not a valid OGG Vorbis file."), *Sound);
				return;
			}
			AsyncTask(ENamedThreads::GameThread, [WeakThis, Decoded = Decoded.ToSharedRef(), Request]() {
				AFINSpeakerPole* This = WeakThis.Get();
				if (!This || This->PlayRequest != Request) return;
				This->StartDecodedSound(Decoded->PCM, Decoded->SampleRate, Decoded->NumChannels);
			});
		});
	}));
}

void AFINSpeakerPole::StartDecodedSound(const TArray<uint8>& PCM, int32 SampleRate, int32 NumChannels) {
	// Starts further in if the sound had to be downloaded or decoded first, so it stays in sync with the server
	const double Elapsed = FMath::Max(0.0, GetServerWorldTime(GetWorld()) - Playback.ServerStartTime);
	const double Position = Playback.StartPoint + Elapsed;

	// Procedural sound waves can't seek, so only the PCM data after the start position gets queued
	const int32 BytesPerFrame = NumChannels * sizeof(int16);
	const int64 TotalFrames = PCM.Num() / BytesPerFrame;
	const int64 StartFrame = FMath::Clamp<int64>(static_cast<int64>(Position * SampleRate), 0, TotalFrames);
	if (StartFrame >= TotalFrames) return;

	USoundWaveProcedural* Wave = NewObject<USoundWaveProcedural>(this);
	Wave->SetSampleRate(SampleRate);
	Wave->NumChannels = NumChannels;
	Wave->Duration = static_cast<float>(TotalFrames - StartFrame) / SampleRate;
	Wave->SoundGroup = SOUNDGROUP_Default;
	// Out of range the sound would get stopped, keep it running silently so it is audible again and in sync when coming back
	Wave->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
	Wave->QueueAudio(PCM.GetData() + StartFrame * BytesPerFrame, static_cast<int32>((TotalFrames - StartFrame) * BytesPerFrame));

	AudioComponent->SetSound(Wave);
	AudioComponent->Play();
	// Procedural sound waves keep playing silence once their data ran out
	GetWorldTimerManager().SetTimer(LocalStopTimer, FTimerDelegate::CreateWeakLambda(this, [this]() {
		AudioComponent->Stop();
	}), Wave->Duration, false);
}

void AFINSpeakerPole::netClass_Meta(FString& InternalName, FText& DisplayName, FText& Description) {
	InternalName = "SpeakerPole";
	DisplayName = FText::FromString("Speaker Pole");
	Description = FText::FromString("This speaker pole allows to play custom sound files, In-Game");
}

void AFINSpeakerPole::netFunc_playSound(const FString& sound, float startPoint) {
	PlaySound(sound, startPoint);
}

void AFINSpeakerPole::netFuncMeta_playSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "playSound";
	DisplayName = FText::FromString("Play Sound");
	Description = FText::FromString("Plays a custom sound file ingame");
	ParameterInternalNames.Add("sound");
	ParameterDisplayNames.Add(FText::FromString("Sound"));
	ParameterDescriptions.Add(FText::FromString("The sound file (without the file ending) you want to play"));
	ParameterInternalNames.Add("startPoint");
	ParameterDisplayNames.Add(FText::FromString("Start Point"));
	ParameterDescriptions.Add(FText::FromString("The start point in seconds at which the system should start playing"));
	Runtime = 0;
}

void AFINSpeakerPole::netFunc_stopSound() {
	StopSound();
}

void AFINSpeakerPole::netFuncMeta_stopSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "stopSound";
	DisplayName = FText::FromString("Stop Sound");
	Description = FText::FromString("Stops the currently playing sound file.");
	Runtime = 0;
}

void AFINSpeakerPole::netSigMeta_SpeakerSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "SpeakerSound";
	DisplayName = FText::FromString("SpeakerSound");
	Description = FText::FromString("Triggers when the sound play state of the speaker pole changes.");
	ParameterInternalNames.Add("type");
	ParameterDisplayNames.Add(FText::FromString("Type"));
	ParameterDescriptions.Add(FText::FromString("The type of the speaker pole event."));
	ParameterInternalNames.Add("sound");
	ParameterDisplayNames.Add(FText::FromString("Sound"));
	ParameterDescriptions.Add(FText::FromString("The sound file including in the event."));
	Runtime = 1;
}
