#include "FINGameInstanceModule.h"

#include "AkAudioDevice.h"
#include "FicsItNetworksModule.h"
#include "Components/FINSpeakerPole.h"
#include "Components/FINSpeakerSoundTransfer.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "FicsItLogLibrary.h"
#include "FicsItNetworksCircuit.h"
#include "FicsItNetworksComputer.h"
#include "FicsItNetworksLuaModule.h"
#include "FIRModModule.h"

UFINGameInstanceModule::UFINGameInstanceModule() {
	RemoteCallObjects.Add(UFINSpeakerRCO::StaticClass());
}

void UFINGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	switch (Phase) {
	case ELifecyclePhase::CONSTRUCTION:
		SpawnChildModule(TEXT("FicsItReflection"), UFIRGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItLogLibrary"), UFILGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksCircuit"), UFINCircuitGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksComputer"), UFINComputerGameInstanceModule::StaticClass());
		SpawnChildModule(TEXT("FicsItNetworksLua"), UFINLuaGameInstanceModule::StaticClass());
		break;
	case ELifecyclePhase::POST_INITIALIZATION:
		RegisterAudioInputPlugin();
		BindSpeakerVolumeConfig();
		break;
	default: break;
	}
}

void UFINGameInstanceModule::BindSpeakerVolumeConfig() {
	UGameInstance* GameInstance = GetGameInstance();
	UConfigManager* ConfigManager = GameInstance ? GameInstance->GetSubsystem<UConfigManager>() : nullptr;
	UConfigPropertySection* Root = ConfigManager ? ConfigManager->GetConfigurationRootSection(FConfigId{TEXT("FicsItNetworks"), TEXT("")}) : nullptr;
	UConfigPropertySection* Audio = Root ? Cast<UConfigPropertySection>(Root->SectionProperties.FindRef(TEXT("Audio"))) : nullptr;
	SpeakerVolumeProperty = Audio ? Cast<UConfigPropertyFloat>(Audio->SectionProperties.FindRef(TEXT("SpeakerVolume"))) : nullptr;
	if (!SpeakerVolumeProperty) return;

	SpeakerVolumeProperty->OnPropertyValueChanged.AddUniqueDynamic(this, &UFINGameInstanceModule::ApplySpeakerVolume);
	ApplySpeakerVolume();
}

void UFINGameInstanceModule::ApplySpeakerVolume() {
	if (!SpeakerVolumeProperty) return;
	AFINSpeakerPole::SetGlobalVolume(SpeakerVolumeProperty->Value);
}

void UFINGameInstanceModule::RegisterAudioInputPlugin() {
	// The speaker plays through the "Wwise Audio Input" source plugin. The game runs the Wwise sound engine as DLL and its
	// Init bank doesn't list this plugin, so it never gets loaded on its own, although AkAudioInput.dll ships with the game.
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice) return;
	const AKRESULT Result = AudioDevice->RegisterPluginDLL(TEXT("AkAudioInput"), TEXT(""));
	UE_LOG(LogFicsItNetworks, Display, TEXT("Registering Wwise plugin AkAudioInput: %d"), static_cast<int32>(Result));
}
