#include "FINSpeakerPole.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"
#include "VorbisAudioInfo.h"
#include "FicsItKernel/Processor/FicsItFuture.h"
#include "FicsItKernel/Processor/Lua/LuaStructs.h"

#include "SML/util/Logging.h"

AFINSpeakerPole::AFINSpeakerPole() {
	NetworkConnector = CreateDefaultSubobject<UFINNetworkConnector>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->OnAudioFinishedNative.AddUObject(this, &AFINSpeakerPole::OnSoundFinished);
}

void AFINSpeakerPole::AddListener_Implementation(FFINNetworkTrace listener) {
	if (Listeners.Contains(listener)) return;
	Listeners.Add(listener);
}

void AFINSpeakerPole::RemoveListener_Implementation(FFINNetworkTrace listener) {
	Listeners.Remove(listener);
}

TSet<FFINNetworkTrace> AFINSpeakerPole::GetListeners_Implementation() {
	return Listeners;
}

UObject* AFINSpeakerPole::GetSignalSenderOverride_Implementation() {
	return NetworkConnector;
}

void AFINSpeakerPole::OnSoundFinished(UAudioComponent* AudioComponent) {
	netSig_SpeakerSound(2, CurrentSound);
	CurrentSound = "";
}

void playSound_Resolve(TSharedRef<FFINDynamicStructHolder> In, TSharedRef<FFINDynamicStructHolder> Out) {
	FFINSpeakersPlaySoundInData& InData = In->Get<FFINSpeakersPlaySoundInData>();
	AFINSpeakerPole* self = InData.Speakers;
	USoundWave* wave = self->LoadSoundFromFile(InData.Sound);
	if (!wave) return;
	if (self->AudioComponent->IsPlaying()) self->netFunc_stopSound();
	self->CurrentSound = InData.Sound;
	self->AudioComponent->SetSound(wave);
	self->AudioComponent->Play(InData.Start);
	self->netSig_SpeakerSound(0, self->CurrentSound);
}
RegisterFuturePointer(playSound_Resolve, playSound_Resolve)

int playSound_Retrieve(lua_State* L, TSharedRef<FFINDynamicStructHolder> Out) {
	return 0;
}
RegisterFuturePointer(playSound_Retrieve, playSound_Retrieve)

FFINNetworkFuture AFINSpeakerPole::netFunc_playSound(const FString& sound, float startPoint) {
	TSharedPtr<FFINDynamicStructHolder> In = MakeShared<FFINDynamicStructHolder>(FFINSpeakersPlaySoundInData::StaticStruct());
	In->Get<FFINSpeakersPlaySoundInData>().Speakers = this;
	In->Get<FFINSpeakersPlaySoundInData>().Sound = sound;
	In->Get<FFINSpeakersPlaySoundInData>().Start = startPoint;
	return FFINNetworkFuture{MakeShared<FicsItKernel::Lua::LuaFutureStruct>(In, nullptr, playSound_Resolve, playSound_Retrieve)};
}

void stopSound_Resolve(TSharedRef<FFINDynamicStructHolder> In, TSharedRef<FFINDynamicStructHolder> Out) {
	FFINSpeakersPlaySoundInData& InData = In->Get<FFINSpeakersPlaySoundInData>();
	AFINSpeakerPole* self = InData.Speakers;
	self->AudioComponent->Stop();
	self->netSig_SpeakerSound(1, self->CurrentSound);
	self->CurrentSound = "";
}
RegisterFuturePointer(stopSound_Resolve, stopSound_Resolve)

FFINNetworkFuture AFINSpeakerPole::netFunc_stopSound() {
	TSharedPtr<FFINDynamicStructHolder> In = MakeShared<FFINDynamicStructHolder>(FFINSpeakersJustSelfInData::StaticStruct());
	In->Get<FFINSpeakersJustSelfInData>().Speakers = this;
	return FFINNetworkFuture{MakeShared<FicsItKernel::Lua::LuaFutureStruct>(In, nullptr, stopSound_Resolve, playSound_Retrieve)};
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
