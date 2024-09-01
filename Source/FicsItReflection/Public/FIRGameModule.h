#pragma once

#include "CoreMinimal.h"
#include "FIRSubsystem.h"
#include "Module/GameWorldModule.h"
#include "FIRGameModule.generated.h"

UCLASS()
class UFIRGameModule : public UGameWorldModule {
	GENERATED_BODY()

	UFIRGameModule() {
		bRootModule = false;
		ModSubsystems.Add(AFIRSubsystem::StaticClass());
	}
};