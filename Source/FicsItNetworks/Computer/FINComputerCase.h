#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "Network/FINNetworkConnector.h"
#include "ModuleSystem/FINModuleSystemPanel.h"

#include "FicsItKernel/FicsItKernel.h"

#include "FINComputerCase.generated.h"

UENUM()
enum EComputerState {
	RUNNING,
	SHUTOFF,
	CRASHED
};

UCLASS(Blueprintable)
class AFINComputerCase : public AFGBuildable {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="ComputerCase")
		UFINNetworkConnector* NetworkConnector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ComputerCase")
		UFINModuleSystemPanel* Panel;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category="ComputerCase")
		FString Code;

	FicsItKernel::KernelSystem* kernel = nullptr;

	AFINComputerCase();
	~AFINComputerCase();

	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin AFGBuildable
	virtual void Factory_Tick(float dt) override;
	// End AFGBuildable

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	UFUNCTION()
	void OnModuleChanged(UObject* module, bool added);

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
		void Toggle();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
		FString GetCrash();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
		EComputerState GetState();

	void recalculateKernelResources();

	UFUNCTION()
	void HandleSignal(FFINSignal signal, FFINNetworkTrace sender);
};
