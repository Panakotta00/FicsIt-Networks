#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "Network/FINNetworkConnector.h"
#include "ModuleSystem/FINModuleSystemPanel.h"

#include "FicsItKernel/FicsItKernel.h"
#include "FicsItKernel/KernelSystemSerializationInfo.h"

#include "FINComputerCase.generated.h"

class AFINComputerDriveHolder;
class AFINComputerMemory;
class AFINComputerProcessor;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category="ComputerCase")
	UFINNetworkConnector* NetworkConnector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category = "ComputerCase")
	UFINModuleSystemPanel* Panel;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category="ComputerCase")
	UFGInventoryComponent* EEPROM;

	UPROPERTY()
	FKernelSystemSerializationInfo KernelState;
	
	FicsItKernel::KernelSystem* kernel = nullptr;

	// Cache
	UPROPERTY()
	TSet<AFINComputerProcessor*> Processors;

	UPROPERTY()
    TSet<AFINComputerMemory*> Memories;

	UPROPERTY()
    TSet<AFINComputerDriveHolder*> DriveHolders;

	AFINComputerCase();
	~AFINComputerCase();

	// Begin UObject
	virtual void Serialize(FArchive& ar) override;
	// End UObject

	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin AFGBuildable
	virtual void Factory_Tick(float dt) override;
	// End AFGBuildable

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual void PreLoadGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	virtual void PostLoadGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	virtual void PreSaveGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	virtual void PostSaveGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	// End IFGSaveInterface

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void AddProcessor(AFINComputerProcessor* processor);

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void RemoveProcessor(AFINComputerProcessor* processor);

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void AddMemory(AFINComputerMemory* memory);

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void RemoveMemory(AFINComputerMemory* memory);

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
	void RecalculateMemory();

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void AddDrive(AFINComputerDriveHolder* DriveHolder);

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void RemoveDrive(AFINComputerDriveHolder* DriveHolder);
	
	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void AddModule(AActor* Module);

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void RemoveModule(AActor* module);

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void AddModules(const TArray<AActor*>& modules);
	
	UFUNCTION()
	void OnModuleChanged(UObject* module, bool added);

	UFUNCTION()
	void OnEEPROMChanged(TSubclassOf<UFGItemDescriptor> Item, int32 Num);

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	void Toggle();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	FString GetCrash();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	EComputerState GetState();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
    void WriteSerialInput(const FString& str);
	
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	FString GetSerialOutput();

	UFUNCTION()
	void HandleSignal(FFINSignal signal, FFINNetworkTrace sender);

private:
	UPROPERTY()
		FString SerialOutput;

	UFUNCTION()
		void OnDriveUpdate(bool added, AFINFileSystemState* drive);
};
