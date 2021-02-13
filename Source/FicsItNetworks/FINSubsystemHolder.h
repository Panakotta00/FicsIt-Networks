#pragma once

#include "CoreMinimal.h"
#include "Computer/FINComputerSubsystem.h"
#include "Network/FINHookSubsystem.h"
#include "Network/Signals/FINSignalSubsystem.h"
#include "SML/mod/ModSubsystems.h"
#include "FINSubsystemHolder.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINSubsystemHolder : public UModSubsystemHolder {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	AFINComputerSubsystem* ComputerSubsystem = nullptr;

	UPROPERTY()
	AFINHookSubsystem* HookSubsystem = nullptr;

	UPROPERTY()
	AFINSignalSubsystem* SignalSubsystem = nullptr;

	// Begin UModSubsystemHolder
	virtual void InitSubsystems() override;
	// End UModSubsystemHolder
};
