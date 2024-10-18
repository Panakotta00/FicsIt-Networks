#pragma once

#include "CoreMinimal.h"
#include "FicsItReflection.h"
#include "FIRGlobalRegisterHelper.h"
#include "FIRHookSubsystem.h"
#include "FIRSubsystem.h"
#include "Module/GameInstanceModule.h"
#include "Module/GameWorldModule.h"
#include "FIRModModule.generated.h"

UCLASS()
class FICSITREFLECTION_API UFIRGameWorldModule : public UGameWorldModule {
	GENERATED_BODY()

	UFIRGameWorldModule() {
		ModSubsystems.Add(AFIRSubsystem::StaticClass());
		ModSubsystems.Add(AFIRHookSubsystem::StaticClass());
	}
};

UCLASS()
class FICSITREFLECTION_API UFIRGameInstanceModule : public UGameInstanceModule {
	GENERATED_BODY()

	// Begin UGameInstanceModule
	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override {
		Super::DispatchLifecycleEvent(Phase);

		if (Phase == ELifecyclePhase::CONSTRUCTION) {
			FFIRGlobalRegisterHelper::Register();

			FFicsItReflectionModule::Get().PopulateSources();
			FFicsItReflectionModule::Get().LoadAllTypes();
		}
	}
	// End UGameInstanceModule
};