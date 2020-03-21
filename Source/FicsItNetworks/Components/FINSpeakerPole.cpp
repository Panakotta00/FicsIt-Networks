#include "FINSpeakerPole.h"

#include <filesystem>
#include <fstream>
#include <sstream>

AFINSpeakerPole::AFINSpeakerPole() {
	NetworkConnector = CreateDefaultSubobject<UFINNetworkConnector>("NetworkConnector");
	NetworkConnector->AddMerged(this);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");

	AudioComponent->OnAudioFinishedNative.AddUObject(this, &AFINSpeakerPole::OnSoundFinished);
}

void AFINSpeakerPole::OnSoundFinished(UAudioComponent* AudioComponent) {
	netSig_SpeakerSound(2, CurrentSound);
	CurrentSound = "";
}

void AFINSpeakerPole::netFunc_PlaySound(const FString& sound, float startPoint) {
	USoundWave* wave = LoadSoundFromFile(sound);
	if (!wave) return;
	if (AudioComponent->IsPlaying()) netFunc_StopSound();
	CurrentSound = sound;
	AudioComponent->SetSound(wave);
	netSig_SpeakerSound(0, CurrentSound);
}

void AFINSpeakerPole::netFunc_StopSound() {
	AudioComponent->Stop();
	netSig_SpeakerSound(1, CurrentSound);
	CurrentSound = "";
}

void AFINSpeakerPole::netSig_SpeakerSound_Implementation(int type, const FString& sound) {}

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

	/*FSoundQualityInfo info;
	FVorbisAudioInfo vorbis_obj = FVorbisAudioInfo();
	if (!vorbis_obj.ReadCompressedInfo((const uint8*) s.data(), (unsigned int)s.size(), &info)) {
		return nullptr;
	}

	sw->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	sw->NumChannels = info.NumChannels;
	sw->Duration = info.Duration;
	sw->RawPCMDataSize = info.SampleDataSize;
	sw->SetSampleRate(info.SampleRate);*/

	sw->bVirtualizeWhenSilent = true;

	return sw;
}
