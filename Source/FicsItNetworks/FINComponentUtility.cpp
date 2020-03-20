#include "FINComponentUtility.h"

#include "WindowsPlatformApplicationMisc.h"
#include "EngineGlobals.h"
#include "VorbisAudioInfo.h"
//#include "Interfaces/IAudioFormat.h"

#include "FGSaveSystem.h"
#include "FGPlayerController.h"

#include "Network/FINNetworkAdapter.h"

#include <filesystem>
#include <fstream>
#include <sstream>

bool UFINComponentUtility::bAllowUsing = true;

UFINNetworkConnector* UFINComponentUtility::GetNetworkConnectorFromHit(FHitResult hit) {
	if (!hit.bBlockingHit) return nullptr;

	UFINNetworkConnector* connector = nullptr;
	FVector pos;

	if (!hit.Actor.IsValid()) return nullptr;
	
	TArray<UActorComponent*> connectors = hit.Actor->GetComponentsByClass(UFINNetworkConnector::StaticClass());

	for (auto con : connectors) {
		if (!Cast<USceneComponent>(con)) continue;

		FVector npos = Cast<USceneComponent>(con)->GetComponentLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = (UFINNetworkConnector*)con;
		}
	}

	if (connector) return connector;
	
	TArray<UActorComponent*> adapters = hit.Actor->GetComponentsByClass(UFINNetworkAdapterReference::StaticClass());
	
	for (auto adapterref : adapters) {
		if (!adapterref || !((UFINNetworkAdapterReference*)adapterref)->Ref) continue;

		FVector npos = ((UFINNetworkAdapterReference*)adapterref)->Ref->GetActorLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = ((UFINNetworkAdapterReference*)adapterref)->Ref->Connector;
		}
	}

	return connector;
}

void UFINComponentUtility::ClipboardCopy(FString str) {
	FWindowsPlatformApplicationMisc::ClipboardCopy(*str);
}

void UFINComponentUtility::SetAllowUsing(UObject* WorldContextObject, bool newUsing) {
	if (!newUsing) {
		// Cast<AFGCharacterPlayer>(WorldContextObject->GetWorld()->GetFirstPlayerController()->GetCharacter())->SetBestUableActor(nullptr);
		// TODO: Find workaround if needed
	}

	bAllowUsing = newUsing;
}

USoundWave* UFINComponentUtility::LoadSoundFromFile(FString file) {
	FString fsp = UFGSaveSystem::GetSaveDirectoryPath();

	file = file + TEXT(".ogg");
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
