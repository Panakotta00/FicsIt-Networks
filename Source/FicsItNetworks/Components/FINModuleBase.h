#pragma once

#include "FGBuildable.h"
#include "ModuleSystem/FINModuleSystemModule.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "Network/FINNetworkCustomType.h"
#include "Network/FINNetworkTrace.h"
#include "Network/Signals/FINSignalSender.h"
#include "FINModuleBase.generated.h"

UCLASS()
class AFINModuleBase : public AFGBuildable, public IFINModuleSystemModule, public IFINSignalSender, public IFINNetworkCustomType {
	GENERATED_BODY()
public:
    /**
    * The signal listeners listening to this component.
    */
    UPROPERTY()
    TSet<FFINNetworkTrace> Listeners;
    
    UPROPERTY(EditDefaultsOnly)
    FVector2D ModuleSize;

	UPROPERTY(EditDefaultsOnly, Replicated)
    FName ModuleName;

	UPROPERTY(BlueprintReadOnly, SaveGame)
    FVector ModulePos;

	UPROPERTY(BlueprintReadOnly, SaveGame)
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

	// Begin IFINNetworkCustomType
	virtual FString GetCustomTypeName_Implementation() const override { return TEXT("ModuleBase"); }
	// End IFINNetworkCustomType
};
