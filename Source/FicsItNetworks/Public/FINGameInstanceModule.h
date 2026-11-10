#pragma once

#include "Misc/Variant.h"
#include "Module/GameInstanceModule.h"
#include "FINGameInstanceModule.generated.h"

UCLASS()
class UFINGameInstanceModule : public UGameInstanceModule {
	GENERATED_BODY()
public:
	UFINGameInstanceModule();

	// Begin UGameInstanceModule
	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;
	// End UGameInstanceModule

private:
	/**
	 * Applies the game's master volume to the Unreal audio device if it changed. The game's volume options only control Wwise,
	 * but the speaker pole, buzzers and panel modules play through Unreal audio.
	 */
	void UpdateMasterVolume();

	float AppliedMasterVolume = -1.f;
};
