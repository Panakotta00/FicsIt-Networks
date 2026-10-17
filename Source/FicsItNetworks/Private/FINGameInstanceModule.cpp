#include "FINGameInstanceModule.h"

#include "FicsItLogLibrary.h"
#include "FicsItNetworksCircuit.h"
#include "FicsItNetworksComputer.h"
#include "FicsItNetworksLuaModule.h"
#include "FIRModModule.h"
#include "FicsItNetworksModule.h"
#include "AudioDevice.h"
#include "FGGameUserSettings.h"
#include "FGOptionInterface.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"

namespace {
	/** Option of the game's master volume (0 to 1), a Wwise RTPC */
	const FString MasterVolumeOption = TEXT("RTPC.Menu_Volume_Master");
}

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
		// The game's option update events never fired for this option in game, so the value gets polled.
		// The options and the audio device also don't exist yet while the game instance initializes.
		if (!IsRunningDedicatedServer()) {
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float) {
				UpdateMasterVolume();
				return true;
			}), 0.5f);
		}
		break;
	default: break;
	}
}

void UFINGameInstanceModule::UpdateMasterVolume() {
	TScriptInterface<IFGOptionInterface> Options = UFGGameUserSettings::GetOptionInterface();
	FAudioDeviceHandle AudioDevice = GEngine ? GEngine->GetMainAudioDevice() : FAudioDeviceHandle();
	if (!Options || !AudioDevice.IsValid()) return;

	float Volume;
	const FVariant Value = Options->GetOptionValue(MasterVolumeOption, FVariant());
	switch (Value.GetType()) {
	case EVariantTypes::Float: Volume = Value.GetValue<float>(); break;
	case EVariantTypes::Double: Volume = static_cast<float>(Value.GetValue<double>()); break;
	// Not loaded yet, don't mute everything
	default: return;
	}
	Volume = FMath::Clamp(Volume, 0.f, 1.f);

	// Applied to the main submix, all Unreal audio passes through it. The transient primary volume of the audio device
	// would be simpler, but the engine resets it on every map load and camera fade.
	// Applied every time, the submix may not exist yet the first time, and it is only a cheap audio thread command.
	AudioDevice->SetSubmixOutputVolume(&AudioDevice->GetMainSubmixObject(), Volume);
	if (Volume != AppliedMasterVolume) {
		AppliedMasterVolume = Volume;
		UE_LOG(LogFicsItNetworks, Display, TEXT("Applied master volume %.2f to Unreal audio."), Volume);
	}
}
