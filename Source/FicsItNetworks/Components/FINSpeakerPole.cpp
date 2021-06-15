#include "FINSpeakerPole.h"

#include "AudioCompressionSettingsUtils.h"
#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"
#include "VorbisAudioInfo.h"
#include "FicsItNetworks/FicsItNetworksModule.h"

AFINSpeakerPole::AFINSpeakerPole() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->SetIsReplicated(true);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->OnAudioFinishedNative.AddUObject(this, &AFINSpeakerPole::OnSoundFinished);
}

UObject* AFINSpeakerPole::GetSignalSenderOverride_Implementation() {
	return NetworkConnector;
}

void AFINSpeakerPole::PlaySound_Implementation(const FString& Sound, float StartPoint) {
	USoundWave* wave = LoadSoundFromFile(Sound);
	if (!wave) return;
	if (AudioComponent->IsPlaying()) netFunc_stopSound();
	CurrentSound = Sound;
	AudioComponent->SetSound(wave);
	AudioComponent->Play(StartPoint);
	netSig_SpeakerSound(0, CurrentSound);
}

void AFINSpeakerPole::StopSound_Implementation() {
	AudioComponent->Stop();
	netSig_SpeakerSound(1, CurrentSound);
	CurrentSound = "";
}

void AFINSpeakerPole::OnSoundFinished(UAudioComponent* InAudioComponent) {
	netSig_SpeakerSound(2, CurrentSound);
	CurrentSound = "";
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

void AFINSpeakerPole::netSig_SpeakerSound_Implementation(int type, const FString& sound) {}

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

USoundWave* AFINSpeakerPole::LoadSoundFromFile(const FString& InSound) {
    FString fsp;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
    if (fsp.IsEmpty()) {
        fsp = FPaths::Combine( FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT( "Saved/" ) TEXT( "SaveGames/" ) );
    }
	
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	
	FString SoundsFolderPath = FPaths::Combine(fsp, TEXT("Computers/Sounds"));

	FileManager.CreateDirectoryTree(*SoundsFolderPath);
	
	FString FilePath = FileManager.ConvertToAbsolutePathForExternalAppForRead(*FPaths::Combine(SoundsFolderPath, InSound + TEXT(".ogg")));
	if (!FilePath.StartsWith(SoundsFolderPath)) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Tried to load sound from '%s' but outside of sounds folder."), *FilePath);
		return nullptr;
	}

	TArray<uint8> DataArray;
	if (!FFileHelper::LoadFileToArray(DataArray, *FilePath)) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Sound file '%s' not found in sounds folder."), *FilePath);
		return nullptr;
	}

	USoundWave* sw = NewObject<USoundWave>();
	if (!sw) return nullptr;

	FName Format = TEXT("OGG");
	const FPlatformAudioCookOverrides* CompressionOverrides = FPlatformCompressionUtilities::GetCookOverrides();
	if (CompressionOverrides) {
		FString HashedString = *Format.ToString();
		FPlatformAudioCookOverrides::GetHashSuffix(CompressionOverrides, HashedString);
		Format = *HashedString;
	}

	FByteBulkData* BulkData = &sw->CompressedFormatData.GetFormat(Format);

	BulkData->Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(BulkData->Realloc(DataArray.Num()), DataArray.GetData(), DataArray.Num());
	BulkData->Unlock();

	FSoundQualityInfo Quality; 
	FVorbisAudioInfo Vorbis;
	if (!Vorbis.ReadCompressedInfo(DataArray.GetData(), DataArray.Num(), &Quality)) return nullptr;
	sw->SetSampleRate(Quality.SampleRate);
	sw->Duration = Quality.Duration;
	sw->NumChannels = Quality.NumChannels;
	sw->RawPCMDataSize = Quality.SampleDataSize;
	sw->SoundGroup = SOUNDGROUP_Default;

	return sw;
}
