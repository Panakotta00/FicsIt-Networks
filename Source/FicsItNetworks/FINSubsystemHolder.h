#pragma once

#include "CoreMinimal.h"
#include "Computer/FINComputerSubsystem.h"
#include "Network/FINHookSubsystem.h"
#include "SML/mod/ModSubsystems.h"
#include "FINSubsystemHolder.generated.h"

UCLASS()
class UFINSubsystemHolder : public UModSubsystemHolder {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	AFINComputerSubsystem* ComputerSubsystem = nullptr;

	UPROPERTY()
	AFINHookSubsystem* HookSubsystem = nullptr;

	// Begin UModSubsystemHolder
	virtual void InitSubsystems() override;
	// End UModSubsystemHolder
};
