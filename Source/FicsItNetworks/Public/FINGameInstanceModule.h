#pragma once

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
	/** Binds the "Audio/SpeakerVolume" config property to the volume of all speaker poles */
	void BindSpeakerVolumeConfig();

	void RegisterAudioInputPlugin();

	UFUNCTION()
	void ApplySpeakerVolume();

	UPROPERTY()
	class UConfigPropertyFloat* SpeakerVolumeProperty = nullptr;
};
