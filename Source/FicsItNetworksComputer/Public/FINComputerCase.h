#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel.h"
#include "Buildables/FGBuildable.h"
#include "FINComputerCase.generated.h"

class AFINComputerNetworkCard;
class AFINComputerDriveHolder;
class AFINComputerMemory;
class AFINComputerProcessor;
class UFILLogContainer;
class UFINModuleSystemPanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINCaseEEPROMUpdateDelegate, const FFGDynamicStruct&, EEPROM);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINCaseFloppyUpdateDelegate, const FGuid&, Floppy);

UCLASS(Blueprintable)
class FICSITNETWORKSCOMPUTER_API AFINComputerCase : public AFGBuildable {
	GENERATED_BODY()
private:
	UPROPERTY(Replicated)
	TEnumAsByte<EFINKernelState> InternalKernelState = FIN_KERNEL_SHUTOFF;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	UFINModuleSystemPanel* Panel = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGInventoryComponent* DataStorage = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAudioComponent* Speaker = nullptr;

	UPROPERTY(SaveGame, Replicated, BlueprintReadOnly)
	UFILLogContainer* Log = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	int LastTabIndex = 0;
	
	UPROPERTY(SaveGame, BlueprintReadOnly)
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
	FGuid Floppy;

	UPROPERTY(BlueprintAssignable)
	FFINCaseEEPROMUpdateDelegate OnEEPROMUpdate;

	UPROPERTY(BlueprintAssignable)
	FFINCaseFloppyUpdateDelegate OnFloppyUpdate;

	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<UObject*> PCIDevices;

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

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnEEPROMChanged(FFIRInstancedStruct state);

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
	void OnEEPROMChanged(int32 Index);

	UFUNCTION(BlueprintCallable)
	FFGDynamicStruct GetEEPROM();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	void Toggle();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	FString GetCrash();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	EFINKernelState GetState();
	
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	AFINComputerProcessor* GetProcessor();

	UFUNCTION()
	void HandleSignal(const FFINSignalData& signal, const FFIRTrace& sender);

	UFUNCTION()
	void OnDriveUpdate(bool bOldLocked, const FGuid& drive);

	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName) {
		InternalName = TEXT("ComputerCase");
		DisplayName = FText::FromString(TEXT("Computer Case"));
	}

	UFUNCTION(BlueprintNativeEvent)
	void netSig_ComputerStateChanged(int64 PrevState, int64 NewState);
	UFUNCTION()
    void netSigMeta_ComputerStateChanged(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "ComputerStateChanged";
		DisplayName = FText::FromString("Computer State Changed");
		Description = FText::FromString("Triggers when the computers state changes.");
		ParameterInternalNames.Add("prevState");
		ParameterDisplayNames.Add(FText::FromString("Previous State"));
		ParameterDescriptions.Add(FText::FromString("The previous computer state."));
		ParameterInternalNames.Add("newState");
		ParameterDisplayNames.Add(FText::FromString("New State"));
		ParameterDescriptions.Add(FText::FromString("The new computer state."));
		Runtime = 1;
	}	

	UFUNCTION(BlueprintNativeEvent)
    void netSig_FileSystemUpdate(int Type, const FString& From, const FString& To);
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

	UFUNCTION()
	int64 netFunc_getState();
	UFUNCTION()
	void netFuncMeta_getState(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getState";
		DisplayName = FText::FromString("Get State");
		Description = FText::FromString("Returns the internal kernel state of the computer.");
		ParameterInternalNames.Add("result");
		ParameterDisplayNames.Add(FText::FromString("Result"));
		ParameterDescriptions.Add(FText::FromString("The current internal kernel state."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_startComputer();
	UFUNCTION()
	void netFuncMeta_startComputer(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "startComputer";
		DisplayName = FText::FromString("Start Computer");
		Description = FText::FromString("Starts the Computer (Processor).");
		Runtime = 0;
	}

	UFUNCTION()
	void netFunc_stopComputer();
	UFUNCTION()
	void netFuncMeta_stopComputer(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "stopComputer";
		DisplayName = FText::FromString("Stop Computer");
		Description = FText::FromString("Stops the Computer (Processor).");
		Runtime = 0;
	}

	UFUNCTION()
	void netFunc_getLog(int64 PageSize, int64 Page, TArray<FFILEntry>& OutLog, int64& OutLogSize);
	UFUNCTION()
	void netFuncMeta_getLog(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getLog";
		DisplayName = FText::FromString("Get Log");
		Description = FText::FromString("Returns the log of the computer. Output is paginated using the input parameters. A negative Page will indicate pagination from the bottom (latest log entry first).");
		ParameterInternalNames.Add("pageSize");
		ParameterDisplayNames.Add(FText::FromString("Page Size"));
		ParameterDescriptions.Add(FText::FromString("The size of the returned page."));
		ParameterInternalNames.Add("page");
		ParameterDisplayNames.Add(FText::FromString("Page"));
		ParameterDescriptions.Add(FText::FromString("The index of the page you want to return. Negative to start indexing at the bottom (latest entries first)."));
		ParameterInternalNames.Add("log");
		ParameterDisplayNames.Add(FText::FromString("Log"));
		ParameterDescriptions.Add(FText::FromString("The Log page you wanted to retrieve."));
		ParameterInternalNames.Add("logSize");
		ParameterDisplayNames.Add(FText::FromString("Log Size"));
		ParameterDescriptions.Add(FText::FromString("The size of the full log (not just the returned page)."));
		Runtime = 0;
	}
};
