#pragma once

#include "FINPciDeviceInterface.h"
#include "FicsItKernel/FicsItKernel.h"
#include "FicsItKernel/Processor/FINStateEEPROM.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "Buildables/FGBuildable.h"
#include "FINComputerCase.generated.h"

class AFINComputerNetworkCard;
class AFINComputerDriveHolder;
class AFINComputerMemory;
class AFINComputerProcessor;
class UFINLog;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINCaseEEPROMUpdateDelegate, AFINStateEEPROM*, EEPROM);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINCaseFloppyUpdateDelegate, AFINFileSystemState*, Floppy);

UCLASS(Blueprintable)
class FICSITNETWORKS_API AFINComputerCase : public AFGBuildable {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	UFINModuleSystemPanel* Panel = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	UFGInventoryComponent* DataStorage = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAudioComponent* Speaker = nullptr;

	UPROPERTY(SaveGame, Replicated)
	FString SerialOutput = "";

	UPROPERTY(SaveGame, Replicated, BlueprintReadOnly)
	UFINLog* Log = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	int LastTabIndex = 0;
	
	UPROPERTY(SaveGame)
	UFINKernelSystem* Kernel = nullptr;

	UPROPERTY(SaveGame)
	UFINKernelNetworkController* NetworkController = nullptr;

	UPROPERTY(SaveGame)
	UFINKernelAudioController* AudioController = nullptr;
	
	// Cache
	UPROPERTY(Replicated)
	TArray<AFINComputerProcessor*> Processors;

	UPROPERTY()
    TSet<AFINComputerMemory*> Memories;

	UPROPERTY()
    TSet<AFINComputerDriveHolder*> DriveHolders;

	UPROPERTY()
	AFINFileSystemState* Floppy = nullptr;

	UPROPERTY(BlueprintAssignable)
	FFINCaseEEPROMUpdateDelegate OnEEPROMUpdate;

	UPROPERTY(BlueprintAssignable)
	FFINCaseFloppyUpdateDelegate OnFloppyUpdate;

	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<UObject*> PCIDevices;

	UPROPERTY(Replicated)
	TEnumAsByte<EFINKernelState> InternalKernelState = FIN_KERNEL_SHUTOFF;

	FString OldSerialOutput = "";

	float KernelTickTime = 0.0;

	AFINComputerCase();
	
	// Begin AActor
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	// End AActor

	// Begin AFGBuildable
	virtual void Factory_Tick(float dt) override;
	// End AFGBuildable

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	virtual void PostLoadGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	// End IFGSaveInterface

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulti_OnEEPROMChanged(AFINStateEEPROM* ChangedEEPROM);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulti_OnFloppyChanged(AFINFileSystemState* ChangedFloppy);

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
	void AddPCIDevice(TScriptInterface<IFINPciDeviceInterface> InPCIDevice);
	
	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
	void RemovePCIDevice(TScriptInterface<IFINPciDeviceInterface> InPCIDevice);
	
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
	EFINKernelState GetState();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
    void WriteSerialInput(const FString& str);
	
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	FString GetSerialOutput();
	
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	AFINComputerProcessor* GetProcessor();

	UFUNCTION()
	void HandleSignal(const FFINSignalData& signal, const FFINNetworkTrace& sender);

	UFUNCTION()
	void OnDriveUpdate(bool bOldLocked, AFINFileSystemState* drive);

	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName) {
		InternalName = TEXT("ComputerCase");
		DisplayName = FText::FromString(TEXT("Computer Case"));
	}

	UFUNCTION()
    void netSig_FileSystemUpdate(int Type, const FString& From, const FString& To) {}
	UFUNCTION()
    void netSigMeta_FileSystemUpdate(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "FileSystemUpdate";
		DisplayName = FText::FromString("File System Update");
		Description = FText::FromString("Triggers when something in the filesystem changes.");
		ParameterInternalNames.Add("type");
		ParameterDisplayNames.Add(FText::FromString("Type"));
		ParameterDescriptions.Add(FText::FromString("The type of the change."));
		ParameterInternalNames.Add("from");
		ParameterDisplayNames.Add(FText::FromString("From"));
		ParameterDescriptions.Add(FText::FromString("The file path to the FS node that has changed."));
		ParameterInternalNames.Add("to");
		ParameterDisplayNames.Add(FText::FromString("To"));
		ParameterDescriptions.Add(FText::FromString("The new file path of the node if it has changed."));
		Runtime = 1;
	}
};
