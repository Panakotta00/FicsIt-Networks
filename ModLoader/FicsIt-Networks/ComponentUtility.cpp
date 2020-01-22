#include "stdafx.h"
#include "ComponentUtility.h"

#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <util/Objects/FVector.h>
#include <assets/AssetFunctions.h>

#include "NetworkAdapter.h"


using namespace SML;
using namespace SML::Objects;

struct Futur {
	std::uint8_t uk[10];
};

struct FUntypedBulkData {
	struct __declspec(align(8)) FAllocatedPtr {
		void *Ptr;
		bool bAllocated;
	};

	void* vfptr;
	unsigned int BulkDataFlags;
	int ElementCount;
	__int64 BulkDataOffsetInFile;
	int BulkDataSizeOnDisk;
	int BulkDataAlignment;
	FUntypedBulkData::FAllocatedPtr BulkData;
	FUntypedBulkData::FAllocatedPtr BulkDataAsync;
	std::uint64_t LockStatus;
	Futur SerializeFuture;
	FString Filename;
	FWeakObjectPtr Package;
};

struct FByteBulkData : FUntypedBulkData {};

struct FThreadSafeCounter {
	volatile int Counter;
};

struct TupleThingy {
	FName key;
	FByteBulkData* val;
};

struct __declspec(align(8)) FFormatContainer {
	TArray<TupleThingy> Formats;
	unsigned int Alignment;
};

struct USoundWave : SDK::USoundBase {
	int CompressionQuality;
	int StreamingPriority;
	char SampleRateQuality[1];
	char DecompressionType;
	char SoundGroup;
	__int8 bLooping : 1;
	__int8 bStreaming : 1;
	__int8 bProcedural : 1;
	__int8 bIsBeginDestroy : 1;
	__int8 bIsBus : 1;
	__int8 bCanProcessAsync : 1;
	__int8 bDynamicResource : 1;
	__int8 bMature : 1;
	__int8 bManualWordWrap : 1;
	__int8 bSingleLine : 1;
	__int8 bVirtualizeWhenSilent : 1;
	__int8 bIsAmbisonics : 1;
	__int8 bDecompressedFromOgg : 1;
	__int8 bCachedSampleRateFromPlatformSettings : 1;
	__int8 bSampleRateManuallyReset : 1;
	char ResourceState[1];
	FThreadSafeCounter PrecacheState;
	FThreadSafeCounter bGenerating; // bool
	float CachedSampleRateOverride;
	FString SpokenText;
	float SubtitlePriority;
	float Volume;
	float Pitch;
	int NumChannels;
	int SampleRate;
	TArray<SDK::FSubtitleCue> Subtitles;
	TArray<SDK::FLocalizedSubtitle> LocalizedSubtitles;
	SDK::UCurveTable *Curves;
	SDK::UCurveTable *InternalCurves;
	void* AudioDecompressor;
	char *CachedRealtimeFirstBuffer;
	char *RawPCMData;
	int RawPCMDataSize;
	char *ResourceData;
	FByteBulkData RawData;
	FGuid CompressedDataGuid;
	FFormatContainer CompressedFormatData;
	int ResourceID;
	int ResourceSize;
	int TrackedMemoryUsage;
	SDK::FStreamedAudioPlatformData *RunningPlatformData;
	TArray<TupleThingy> CookedPlatformData;
};


bool UComponentUtility::allowUsing = true;

UNetworkConnector* UComponentUtility::getNetworkConnectorFromHit(SDK::FHitResult hit) {
	if (!hit.bBlockingHit) return nullptr;

	UNetworkConnector* connector = nullptr;
	FVector pos;

	auto obj = UObject::GetObjectCasted<SDK::AActor>(FWeakObjectPtr(hit.Actor).index);
	if (!obj) return nullptr;
	
	TArray<SDK::UActorComponent*> connectors = obj->GetComponentsByClass((SDK::UClass*)UNetworkConnector::staticClass());

	for (auto con : connectors) {
		if (!con) continue;

		FVector npos = ((SDK::USceneComponent*) con)->K2_GetComponentToWorld().Translation;
		if (!connector || (pos - hit.ImpactPoint).length() > (npos - hit.ImpactPoint).length()) {
			pos = npos;
			connector = (UNetworkConnector*)con;
		}
	}

	if (connector) return connector;
	
	TArray<SDK::UActorComponent*> adapters = obj->GetComponentsByClass((SDK::UClass*)UNetworkAdapterReference::staticClass());
	
	for (auto adapterref : adapters) {
		if (!adapterref || !((UNetworkAdapterReference*)adapterref)->ref) continue;

		FVector npos = ((UNetworkAdapterReference*)adapterref)->ref->K2_GetActorLocation();
		if (!connector || (pos - hit.ImpactPoint).length() > (npos - hit.ImpactPoint).length()) {
			pos = npos;
			connector = ((UNetworkAdapterReference*)adapterref)->ref->connector;
		}
	}

	return connector;
}

void UComponentUtility::connectPower(void * self, FFrame & stack, void * ret) {
	static void(*addHiddenConnection)(SDK::UFGPowerConnectionComponent*, SDK::UFGPowerConnectionComponent*) = nullptr;
	if (!addHiddenConnection) addHiddenConnection = (void(*)(SDK::UFGPowerConnectionComponent*, SDK::UFGPowerConnectionComponent*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGCircuitConnectionComponent::AddHiddenConnection");

	SDK::UFGPowerConnectionComponent* comp1;
	SDK::UFGPowerConnectionComponent* comp2;

	stack.stepCompIn(&comp1);
	stack.stepCompIn(&comp2);

	stack.code += !!stack.code;

	if (comp1, comp2) addHiddenConnection(comp1, comp2);
}

void UComponentUtility::disconnectPower(void * self, FFrame & stack, void * ret) {
	static void(*removeHiddenConnection)(SDK::UFGPowerConnectionComponent*, SDK::UFGPowerConnectionComponent*) = nullptr;
	if (!removeHiddenConnection) removeHiddenConnection = (void(*)(SDK::UFGPowerConnectionComponent*, SDK::UFGPowerConnectionComponent*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGCircuitConnectionComponent::RemoveHiddenConnection");

	SDK::UFGPowerConnectionComponent* comp1 = nullptr;
	SDK::UFGPowerConnectionComponent* comp2 = nullptr;

	stack.stepCompIn(&comp1);
	stack.stepCompIn(&comp2);

	stack.code += !!stack.code;

	if (comp1 && comp2) {
		auto cons = (TArray<SDK::UFGCircuitConnectionComponent*>*)&comp1->mHiddenConnections;
		for (int i = 0; i < cons->num(); ++i) if ((*cons)[i] == comp2) {
			removeHiddenConnection(comp1, comp2);
			return;
		}
	}
}

void UComponentUtility::getNetworkConnectorFromHit_exec(void * self, SML::Objects::FFrame & stack, void * ret) {
	SDK::FHitResult hit;
	auto& con = *(UNetworkConnector**)ret;

	stack.stepCompIn(&hit);
	
	stack.code += !!stack.code;

	con = getNetworkConnectorFromHit(hit);
}

void UComponentUtility::clipboardCopy(void * self, SML::Objects::FFrame & stack, void * ret) {
	static void(*cpyClip)(const wchar_t*) = nullptr;
	if (!cpyClip) cpyClip = (void(*)(const wchar_t*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FWindowsPlatformApplicationMisc::ClipboardCopy");

	FString str;

	stack.stepCompIn(&str);

	stack.code += !!stack.code;

	std::wstring t(str.c_str());

	cpyClip(t.c_str());
}

void UComponentUtility::setAllowUsing(void * self, SML::Objects::FFrame & stack, void * ret) {
	static void(*setBestUsable)(SDK::AFGCharacterBase*, SDK::AActor*) = nullptr;
	if (!setBestUsable) setBestUsable = (void(*)(SDK::AFGCharacterBase*, SDK::AActor*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "AFGCharacterPlayer::SetBestUsableActor");

	bool newUsing = false;
	stack.stepCompIn(&newUsing);

	stack.code += !!stack.code;

	if (!newUsing) {
		auto c = SML::Mod::Functions::getPlayerCharacter();
		setBestUsable(c, nullptr);
	}

	allowUsing = newUsing;
}

void UComponentUtility::dumpObject(void * self, SML::Objects::FFrame & stack, void * ret) {
	SML::Objects::UObject* obj = nullptr;
	stack.stepCompIn(&obj);

	stack.code += !!stack.code;

	for (auto f : *obj->clazz) {
		std::string s = f->getName() + " " + f->clazz->getName() + " ";
		if (f->clazz->castFlags & EClassCastFlags::CAST_UProperty) {
			if (f->clazz->castFlags & EClassCastFlags::CAST_UObjectProperty) {
				s += ((SML::Objects::UObjectProperty*)f)->objClass->getName() + " ";
			}
			if (((UProperty*)f)->propFlags & EPropertyFlags::Prop_ExposeOnSpawn) s += "ExposeOnSpawn ";
			if (((UProperty*)f)->propFlags & EPropertyFlags::Prop_Protected) s += "Protected ";
			if (((UProperty*)f)->propFlags & EPropertyFlags::Prop_BlueprintReadOnly) s += "ReadOnly ";
			if (((UProperty*)f)->propFlags & EPropertyFlags::Prop_Edit) s += "Edit ";
			Utility::debug(s);
		} else if (f->clazz->castFlags & EClassCastFlags::CAST_UFunction) {
			s += "(";
			SML::Objects::UField* p = ((SML::Objects::UFunction*)f)->childs;
			bool was = p;
			while (p) {
				if (p->clazz->castFlags & EClassCastFlags::CAST_UProperty && ((SML::Objects::UProperty*)p)->propFlags & (EPropertyFlags::Prop_OutParm | EPropertyFlags::Prop_Parm | EPropertyFlags::Prop_ReturnParm)) {
					s += p->getName() + " " + p->clazz->getName() + ", ";
				}
				p = p->next;
			}
			if (was) s = s.substr(0, s.length() - 2);
			s += ") ";
			Utility::warning(s);
		}
	}
}

struct FSoundQualityInfo {
	int Quality;
	unsigned int NumChannels;
	unsigned int SampleRate;
	unsigned int SampleDataSize;
	float Duration;
	FString DebugName;
};

struct ICompressedAudioInfo {
	void *vfptr;
};

struct __declspec(align(8)) FVorbisAudioInfo : ICompressedAudioInfo {
	void *VFWrapper;
	const char *SrcBufferData;
	unsigned int SrcBufferDataSize;
	unsigned int BufferOffset;
	FThreadSafeCounter bPerformingOperation;
	FWindowsCriticalSection VorbisCriticalSection;
	USoundWave *StreamingSoundWave;
	unsigned int StreamingChunksSize;

	FVorbisAudioInfo() {
		((void(*)(void*))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FVorbisAudioInfo::FVorbisAudioInfo"))(this);
	}

	~FVorbisAudioInfo() {
		((void(*)(void*))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FVorbisAudioInfo::~FVorbisAudioInfo"))(this);
	}
};


void UComponentUtility::loadSoundFromFile(SML::Objects::FFrame & stack, void * ret) {
	static FString*(*getSavePath)(FString*) = nullptr;
	if (!getSavePath) getSavePath = (FString*(*)(FString*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGSaveSystem::GetSaveDirectoryPath");
	
	FString file;
	stack.stepCompIn(&file);
	stack.code += !!stack.code;
	auto& sound = *(USoundWave**)ret;

	FString fsp;
	getSavePath(&fsp);

	std::string file_s = file.toStr() + ".ogg";
	auto path = std::filesystem::path(file_s);
	while (path.is_absolute()) path = file_s.substr(1);
	std::filesystem::path root = fsp.toStr();
	root /= "Computers/Sounds";
	std::filesystem::create_directories(root);
	std::filesystem::path p = root / path;
	p = std::filesystem::absolute(p);
	auto ps = p.string();
	if (ps.rfind(std::filesystem::absolute(root).string(), 0) != 0 || !std::filesystem::exists(p)) {
		sound = nullptr;
		return;
	}

	USoundWave* sw = (USoundWave*)((SML::Objects::UClass*)SDK::USoundWave::StaticClass())->newObj();
	if (!sw) {
		sound = nullptr;
		return;
	}
	bool loaded = false;
	std::fstream f;
	f.open(p, std::fstream::in | std::fstream::binary);
	std::stringstream strs;
	strs << f.rdbuf();
	auto s = strs.str();
	FByteBulkData* data = ((FByteBulkData*(*)(FFormatContainer*, FName))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FFormatContainer::GetFormat"))(&sw->CompressedFormatData, L"OGG");
	((void*(*)(void*, unsigned int))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FUntypedBulkData::Lock"))(data, 0x2);
	memcpy(((void*(*)(FByteBulkData*, int))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FUntypedBulkData::Realloc"))(data, (int)s.size()), s.data(), s.size());
	((void(*)(void*))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FUntypedBulkData::Unlock"))(data);

	FSoundQualityInfo info;
	FVorbisAudioInfo vorbis_obj = FVorbisAudioInfo();
	if (!(((bool(*)(void*,void*,unsigned int,void*))DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FVorbisAudioInfo::ReadCompressedInfo"))(&vorbis_obj, s.data(), (unsigned int)s.size(), &info))) {
		sound = nullptr;
		return;
	}

	sw->SoundGroup = (char)SDK::ESoundGroup::SOUNDGROUP_Default;
	sw->NumChannels = info.NumChannels;
	sw->Duration = info.Duration;
	sw->RawPCMDataSize = info.SampleDataSize;
	sw->SampleRate = info.SampleRate;

	sw->bVirtualizeWhenSilent = true;

	sound = sw;
}
