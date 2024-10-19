#pragma once

#include "Module/GameWorldModule.h"
#include "FINGameWorldModule.generated.h"

UCLASS()
class UFINGameWorldModule : public UGameWorldModule {
	GENERATED_BODY()
public:
	UFINGameWorldModule();

	// Begin UGameInstanceModule
	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;
	// End UGameInstanceModule
};
