#include "Components/FINSpeakerPole.h"
#include "FicsItNetworksModule.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "Async/Async.h"
#include "Components/AudioComponent.h"
#include "Components/FINSpeakerAudioComponent.h"
#include "Components/FINSpeakerSoundTransfer.h"
#include "Decoders/VorbisAudioInfo.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace {
	double GetServerWorldTime(const UWorld* World) {
		const AGameStateBase* GameState = World->GetGameState();
		return GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
	}
}

AFINSpeakerPole::AFINSpeakerPole() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->SetIsReplicated(true);

	SpeakerAudio = CreateDefaultSubobject<UFINSpeakerAudioComponent>("SpeakerAudio");
	SpeakerAudio->SetupAttachment(RootComponent);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;
}

void AFINSpeakerPole::BeginPlay() {
	Super::BeginPlay();

	SpeakerAudio->EnsureEventLoaded();
	SoundAvailableHandle = FFINSpeakerSoundTransfer::OnSoundAvailable.AddUObject(this, &AFINSpeakerPole::OnSoundAvailable);
	ApplyAudioSettings();
}

void AFINSpeakerPole::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	FFINSpeakerSoundTransfer::OnSoundAvailable.Remove(SoundAvailableHandle);
	GetWorldTimerManager().ClearTimer(FinishedTimer);
	GetWorldTimerManager().ClearTimer(DownloadRetryTimer);
	++PlayRequest;
	SpeakerAudio->StopData();
	PlaybackData.Reset();
	Super::EndPlay(EndPlayReason);
}

void AFINSpeakerPole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFINSpeakerPole, Range);
	DOREPLIFETIME(AFINSpeakerPole, Volume);
	DOREPLIFETIME(AFINSpeakerPole, Playback);
}

UObject* AFINSpeakerPole::GetSignalSenderOverride_Implementation() {
	return NetworkConnector;
}

void AFINSpeakerPole::PlaySound(const FString& Sound, float StartPoint) {
	if (!HasAuthority()) return;

	TArray<uint8> FileData;
	if (!LoadSoundFile(Sound, FileData)) return;
	if (FileData.Num() > FFINSpeakerSoundTransfer::MaxFileSize) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Sound file '%s' is too large (%d bytes, max %d bytes)."), *Sound, FileData.Num(), FFINSpeakerSoundTransfer::MaxFileSize);
		return;
	}

	// Only reads the header, a dedicated server doesn't decode the sound
	FSoundQualityInfo Quality;
	FVorbisAudioInfo Vorbis;
	if (!Vorbis.ReadCompressedInfo(FileData.GetData(), FileData.Num(), &Quality) || Quality.NumChannels == 0 || Quality.SampleRate == 0) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Sound file '%s' is not a valid OGG Vorbis file."), *Sound);
		return;
	}

	const int32 Size = FileData.Num();
	const FString Hash = FFINSpeakerSoundTransfer::Add(MoveTemp(FileData));
	// Keeps the file in the cache while clients may still download it
	PlaybackData = FFINSpeakerSoundTransfer::Find(Hash);

	Playback.Sound = Sound;
	Playback.Hash = Hash;
	Playback.Size = Size;
	Playback.StartPoint = FMath::Max(0.f, StartPoint);
	Playback.Duration = Quality.Duration;
	Playback.ServerStartTime = GetServerWorldTime(GetWorld());
	FlushNetDormancy();
	ForceNetUpdate();

	CurrentSound = Sound;
	netSig_SpeakerSound(0, CurrentSound);

	// The server decides when the sound finished, a dedicated server doesn't play it
	const float Remaining = Playback.Duration - Playback.StartPoint;
	if (Remaining > 0.f) {
		GetWorldTimerManager().SetTimer(FinishedTimer, this, &AFINSpeakerPole::OnSoundFinished, Remaining, false);
		PlayLocally();
	} else {
		GetWorldTimerManager().ClearTimer(FinishedTimer);
		PlayLocally();
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

void AFINSpeakerPole::OnRep_AudioSettings() {
	ApplyAudioSettings();
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
	GetWorldTimerManager().ClearTimer(DownloadRetryTimer);

	if (!Playback.IsSet()) {
		SpeakerAudio->StopData();
		return;
	}
	// No audio on a dedicated server
	if (GetNetMode() == NM_DedicatedServer) return;

	const double Elapsed = GetServerWorldTime(GetWorld()) - Playback.ServerStartTime;
	if (Playback.StartPoint + Elapsed >= Playback.Duration) {
		SpeakerAudio->StopData();
		return;
	}

	TSharedPtr<const TArray<uint8>> FileData = FFINSpeakerSoundTransfer::Find(Playback.Hash);
	if (!FileData.IsValid()) {
		// Use the file of the own sounds folder if it is the same as the server's
		TArray<uint8> LocalData;
		if (LoadSoundFile(Playback.Sound, LocalData) && LocalData.Num() == Playback.Size && FFINSpeakerSoundTransfer::HashData(LocalData) == Playback.Hash) {
			FFINSpeakerSoundTransfer::Add(MoveTemp(LocalData));
			FileData = FFINSpeakerSoundTransfer::Find(Playback.Hash);
		}
	}
	if (!FileData.IsValid()) {
		// Stop the previous sound while downloading the new one
		SpeakerAudio->StopData();
		if (!FFINSpeakerSoundTransfer::Request(GetWorld(), Playback.Hash, Playback.Size)) {
			// The remote call object of the player is not replicated yet right after joining
			GetWorldTimerManager().SetTimer(DownloadRetryTimer, this, &AFINSpeakerPole::PlayLocally, 1.f, false);
		}
		return;
	}

	// Decoding a whole file takes a moment, don't block the game thread
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis = TWeakObjectPtr<AFINSpeakerPole>(this), FileData = FileData.ToSharedRef(), Sound = Playback.Sound, Request]() {
		TSharedPtr<const FFINSpeakerSoundData> Decoded = UFINSpeakerAudioComponent::DecodeOgg(*FileData);
		if (!Decoded.IsValid()) {
			UE_LOG(LogFicsItNetworks, Warning, TEXT("Sound file '%s' is not a valid OGG Vorbis file."), *Sound);
			return;
		}
		AsyncTask(ENamedThreads::GameThread, [WeakThis, Decoded = Decoded.ToSharedRef(), Request]() {
			AFINSpeakerPole* This = WeakThis.Get();
			if (!This || This->PlayRequest != Request) return;
			// Starts further in if the sound had to be downloaded or decoded first, so it stays in sync with the server
			const double Elapsed = FMath::Max(0.0, GetServerWorldTime(This->GetWorld()) - This->Playback.ServerStartTime);
			This->ApplyAudioSettings();
			This->SpeakerAudio->PlayData(Decoded, This->Playback.StartPoint + Elapsed);
		});
	});
}

void AFINSpeakerPole::OnSoundAvailable(const FString& Hash) {
	if (Playback.Hash == Hash) {
		PlayLocally();
	}
}

void AFINSpeakerPole::ApplyAudioSettings() {
	// The Wwise attenuation fades to silence at BaseAttenuationRange, the scaling factor stretches it to the configured range
	SpeakerAudio->SetAttenuationScalingFactor(FMath::Max(Range, 0.01f) / BaseAttenuationRange);
	// The speaker outputs to the game's Master Audio Bus (a mod can't add busses, they live in the game's Init bank),
	// so the FIN volume setting is applied per speaker
	SpeakerAudio->SetRTPCValue(nullptr, Volume, 0, TEXT("FIN_Speaker_Gain"));
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
	Description = FText::FromString("Plays a custom sound file ingame. The file has to be an OGG Vorbis file (.ogg) in the Computers/Sounds folder of the save games directory.");
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

void AFINSpeakerPole::netFunc_setRange(float InRange) {
	const float OldRange = Range;
	Range = FMath::Max(InRange, 0.f);
	ApplyAudioSettings();
	FlushNetDormancy();
	ForceNetUpdate();
	netSig_SpeakerSetting(0, Range, OldRange);
}

void AFINSpeakerPole::netFuncMeta_setRange(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "setRange";
	DisplayName = FText::FromString("Set Range");
	Description = FText::FromString("Sets the range in meters at which the sound of this speaker fades out to silence.");
	ParameterInternalNames.Add("range");
	ParameterDisplayNames.Add(FText::FromString("Range"));
	ParameterDescriptions.Add(FText::FromString("The range in meters"));
	Runtime = 0;
}

float AFINSpeakerPole::netFunc_getRange() {
	return Range;
}

void AFINSpeakerPole::netFuncMeta_getRange(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "getRange";
	DisplayName = FText::FromString("Get Range");
	Description = FText::FromString("Returns the range in meters at which the sound of this speaker fades out to silence.");
	ParameterInternalNames.Add("range");
	ParameterDisplayNames.Add(FText::FromString("Range"));
	ParameterDescriptions.Add(FText::FromString("The range in meters"));
	Runtime = 1;
}

void AFINSpeakerPole::netFunc_setVolume(float InVolume) {
	const float OldVolume = Volume;
	Volume = FMath::Clamp(InVolume, 0.f, MaxVolume);
	ApplyAudioSettings();
	FlushNetDormancy();
	ForceNetUpdate();
	netSig_SpeakerSetting(1, Volume, OldVolume);
}

void AFINSpeakerPole::netFuncMeta_setVolume(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "setVolume";
	DisplayName = FText::FromString("Set Volume");
	Description = FText::FromString("Sets the volume of this speaker. 0 is silent, 1 is the original volume of the sound file, values up to 4 amplify the sound (might distort).");
	ParameterInternalNames.Add("volume");
	ParameterDisplayNames.Add(FText::FromString("Volume"));
	ParameterDescriptions.Add(FText::FromString("The volume multiplier (0 to 4)"));
	Runtime = 0;
}

float AFINSpeakerPole::netFunc_getVolume() {
	return Volume;
}

void AFINSpeakerPole::netFuncMeta_getVolume(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "getVolume";
	DisplayName = FText::FromString("Get Volume");
	Description = FText::FromString("Returns the volume multiplier of this speaker (0 to 4).");
	ParameterInternalNames.Add("volume");
	ParameterDisplayNames.Add(FText::FromString("Volume"));
	ParameterDescriptions.Add(FText::FromString("The volume multiplier"));
	Runtime = 1;
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

void AFINSpeakerPole::netSigMeta_SpeakerSetting(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "SpeakerSetting";
	DisplayName = FText::FromString("Speaker Setting");
	Description = FText::FromString("Triggers when the range or the volume of the speaker pole changes.");
	ParameterInternalNames.Add("setting");
	ParameterDisplayNames.Add(FText::FromString("Setting"));
	ParameterDescriptions.Add(FText::FromString("The changed setting. 0 = range, 1 = volume"));
	ParameterInternalNames.Add("new");
	ParameterDisplayNames.Add(FText::FromString("New"));
	ParameterDescriptions.Add(FText::FromString("The new value of the setting"));
	ParameterInternalNames.Add("old");
	ParameterDisplayNames.Add(FText::FromString("Old"));
	ParameterDescriptions.Add(FText::FromString("The previous value of the setting"));
	Runtime = 1;
}

bool AFINSpeakerPole::LoadSoundFile(const FString& InSound, TArray<uint8>& OutData) {
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	const FString SaveGamesPath = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/SaveGames/"));

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	const FString SoundsFolderPath = FPaths::Combine(SaveGamesPath, TEXT("Computers/Sounds"));
	FileManager.CreateDirectoryTree(*SoundsFolderPath);

	FString FilePath = FPaths::Combine(SoundsFolderPath, InSound + TEXT(".ogg"));
	FPaths::CollapseRelativeDirectories(FilePath);
	if (!FilePath.StartsWith(SoundsFolderPath + TEXT("/"))) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Tried to load sound from '%s' but outside of sounds folder."), *FilePath);
		return false;
	}

	if (!FFileHelper::LoadFileToArray(OutData, *FilePath)) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Sound file '%s' not found in sounds folder."), *FilePath);
		return false;
	}
	return true;
}
