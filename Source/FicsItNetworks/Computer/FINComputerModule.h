#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "ModuleSystem/FINModuleSystemModule.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "Network/Signals/FINSignalSender.h"

#include "FINComputerModule.generated.h"

UCLASS()
class AFINComputerModule : public AFGBuildable, public IFINModuleSystemModule, public IFINSignalSender {
	GENERATED_BODY()

public:
	/**
    * The signal listeners listening to this component.
    */
    UPROPERTY()
    TSet<FFINNetworkTrace> Listeners;
    
	UPROPERTY(EditDefaultsOnly, Category="ComputerModule")
	FVector2D ModuleSize;

	UPROPERTY(EditDefaultsOnly, Category="ComputerModule")
	FName ModuleName;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category="ComputerModule")
	FVector ModulePos;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category="ComputerModule")
	UFINModuleSystemPanel* ModulePanel = nullptr;

	// Begin UObject
	virtual void Serialize(FArchive& Ar) override;
	// End UObject

	// Begin AActor
	virtual void EndPlay(EEndPlayReason::Type reason) override;
	// End AActor
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFINModuleSystemModule
	virtual void setPanel_Implementation(UFINModuleSystemPanel* Panel, int X, int Y, int Rot) override;
	virtual void getModuleSize_Implementation(int& Width, int& Height) const override;
	virtual FName getName_Implementation() const override;
	// End IFINModuleSystemModule

	// Begin IFINSignalSender
	virtual void AddListener_Implementation(FFINNetworkTrace listener) override;
	virtual void RemoveListener_Implementation(FFINNetworkTrace listener) override;
	virtual TSet<FFINNetworkTrace> GetListeners_Implementation() override;
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender
};