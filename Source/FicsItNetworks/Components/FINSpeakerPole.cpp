#include "FINSpeakerPole.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"
#include "VorbisAudioInfo.h"
#include "FicsItKernel/Processor/Lua/LuaStructs.h"

AFINSpeakerPole::AFINSpeakerPole() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);

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

void AFINSpeakerPole::OnSoundFinished(UAudioComponent* AudioComponent) {
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
	Runtime = 2;
}

void AFINSpeakerPole::netFunc_stopSound() {
	StopSound();
}

void AFINSpeakerPole::netFuncMeta_stopSound(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
	InternalName = "stopSound";
	DisplayName = FText::FromString("Stop Sound");
	Description = FText::FromString("Stops the currently playing sound file.");
	Runtime = 2;
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
	Runtime = 2;
}

USoundWave* AFINSpeakerPole::LoadSoundFromFile(const FString& sound) {
	FString fsp = UFGSaveSystem::GetSaveDirectoryPath();

	auto file = sound + TEXT(".ogg");
	auto path = std::experimental::filesystem::path(*file);

	std::experimental::filesystem::path root = *fsp;
	root /= "Computers/Sounds";
	std::experimental::filesystem::create_directories(root);
	auto pathToFile = root / path;
	pathToFile = std::experimental::filesystem::absolute(pathToFile);
	auto ps = pathToFile.string();
	if (ps.rfind(std::experimental::filesystem::absolute(root).string(), 0) != 0 || !std::experimental::filesystem::exists(pathToFile)) {
		return nullptr;
	}

	USoundWave* sw = NewObject<USoundWave>();
	if (!sw) return nullptr;
	bool loaded = false;
	std::fstream f;
	f.open(pathToFile, std::fstream::in | std::fstream::binary);
	std::stringstream strs;
	strs << f.rdbuf();
	auto s = strs.str();
	FByteBulkData& data = sw->CompressedFormatData.GetFormat(L"OGG");
	data.Lock(0x2);
	memcpy(data.Realloc(s.size()), s.data(), s.size());
	data.Unlock();
	
	FSoundQualityInfo info;
	FVorbisAudioInfo* vorbis_obj = new FVorbisAudioInfo();
	if (!vorbis_obj->ReadCompressedInfo((const uint8*) s.data(), (unsigned int)s.size(), &info)) {
		return nullptr;
	}

	sw->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	sw->NumChannels = info.NumChannels;
	sw->Duration = info.Duration;
	sw->RawPCMDataSize = info.SampleDataSize;
	sw->SetSampleRate(info.SampleRate);

	sw->bVirtualizeWhenSilent = true;

	delete vorbis_obj;

	return sw;
}
