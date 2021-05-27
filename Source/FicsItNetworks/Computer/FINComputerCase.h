#pragma once

#include "CoreMinimal.h"
#include "FINComputerGPU.h"
#include "FINComputerScreen.h"
#include "FINInternetCard.h"
#include "Buildables/FGBuildable.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
#include "FicsItNetworks/ModuleSystem/FINModuleSystemPanel.h"
#include "FicsItNetworks/FicsItKernel/FicsItKernel.h"
#include "FicsItNetworks/FicsItKernel/Processor/FINStateEEPROM.h"

#include "FINComputerCase.generated.h"

class AFINComputerNetworkCard;
class AFINComputerDriveHolder;
class AFINComputerMemory;
class AFINComputerProcessor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINCaseEEPROMUpdateDelegate, AFINStateEEPROM*, EEPROM);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINCaseFloppyUpdateDelegate, AFINFileSystemState*, Floppy);

UCLASS(Blueprintable)
class FICSITNETWORKS_API AFINComputerCase : public AFGBuildable {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	UFINModuleSystemPanel* Panel = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	UFGInventoryComponent* DataStorage = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAudioComponent* Speaker = nullptr;

	UPROPERTY(SaveGame, Replicated)
	FString SerialOutput = "";

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
	TSet<AFINComputerNetworkCard*> NetworkCards;

	UPROPERTY()
	AFINFileSystemState* Floppy = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TArray<AFINComputerScreen*> Screens;

	UPROPERTY(BlueprintAssignable)
	FFINCaseEEPROMUpdateDelegate OnEEPROMUpdate;

	UPROPERTY(BlueprintAssignable)
	FFINCaseFloppyUpdateDelegate OnFloppyUpdate;

	UPROPERTY(Replicated)
	TEnumAsByte<EFINKernelState> InternalKernelState = FIN_KERNEL_SHUTOFF;

	FString OldSerialOutput = "";

	float KernelTickTime = 0.0;

	AFINComputerCase();
	
	// Begin UObject
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End UObject

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
	void AddGPU(AFINComputerGPU* GPU);
	
	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
	void RemoveGPU(AFINComputerGPU* GPU);
	
	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
	void AddScreen(AFINComputerScreen* Screen);
	
	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
	void RemoveScreen(AFINComputerScreen* Screen);

	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void AddNetCard(AFINComputerNetworkCard* NetCard);
	
	UFUNCTION(BlueprintCallable, Category = "Network|Computer")
    void RemoveNetCard(AFINComputerNetworkCard* NetCard);
	
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

	UFUNCTION()
	AFINInternetCard* netFunc_getINetCard() {
		TArray<AActor*> Modules;
		Panel->GetModules(Modules);
		for (AActor* Actor : Modules) {
			if (Actor->IsA<AFINInternetCard>()) return Cast<AFINInternetCard>(Actor);
		}
		return nullptr;
	}
};
