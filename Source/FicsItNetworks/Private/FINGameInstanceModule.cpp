#include "FINGameInstanceModule.h"

#include "AkAudioDevice.h"
#include "FicsItNetworksModule.h"
#include "FicsItLogLibrary.h"
#include "FicsItNetworksCircuit.h"
#include "FicsItNetworksComputer.h"
#include "FicsItNetworksLuaModule.h"
#include "FIRModModule.h"

UFINGameInstanceModule::UFINGameInstanceModule() {}

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
		break;
	default: break;
	}
}

void UFINGameInstanceModule::RegisterAudioInputPlugin() {
	// The speaker plays through the "Wwise Audio Input" source plugin. The game runs the Wwise sound engine as DLL and its
	// Init bank doesn't list this plugin, so it never gets loaded on its own, although AkAudioInput.dll ships with the game.
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice) return;
	const AKRESULT Result = AudioDevice->RegisterPluginDLL(TEXT("AkAudioInput"), TEXT(""));
	UE_LOG(LogFicsItNetworks, Display, TEXT("Registering Wwise plugin AkAudioInput: %d"), static_cast<int32>(Result));
}
