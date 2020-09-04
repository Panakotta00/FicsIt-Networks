#pragma once

#include "CoreMinimal.h"

#include "FGReplicationDetailInventoryComponent.h"
#include "FINComputerGPU.h"
#include "FINComputerScreen.h"
#include "Buildables/FGBuildable.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "ModuleSystem/FINModuleSystemPanel.h"

#include "FicsItKernel/FicsItKernel.h"
#include "FicsItKernel/KernelSystemSerializationInfo.h"
#include "FicsItKernel/Audio/AudioComponentController.h"
#include "Network/FINNetworkCustomType.h"

#include "FINComputerCase.generated.h"

class AFINComputerNetworkCard;
class AFINComputerDriveHolder;
class AFINComputerMemory;
class AFINComputerProcessor;

UENUM()
enum EComputerState {
	RUNNING,
	SHUTOFF,
	CRASHED
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINCaseEEPROMUpdateDelegate, AFINStateEEPROM*, EEPROM);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINCaseFloppyUpdateDelegate, AFINFileSystemState*, Floppy);

UCLASS(Blueprintable)
class AFINComputerCase : public AFGBuildable, public IFINNetworkCustomType {
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

	UPROPERTY()
	UFINAudioComponentControllerTrampoline* SpeakerTrampoline = nullptr;

	UPROPERTY(SaveGame, Replicated)
	FString SerialOutput = "";

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	int LastTabIndex = 0;
	
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

	FString OldSerialOutput = "";

	float KernelTickTime = 0.0;

	AFINComputerCase();
	~AFINComputerCase();

	// Begin UObject
	virtual void Serialize(FArchive& ar) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End UObject

	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	// End AActor

	// Begin AFGBuildable
	virtual void Factory_Tick(float dt) override;
	// End AFGBuildable

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	virtual void PreLoadGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	virtual void PostLoadGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	virtual void PreSaveGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	virtual void PostSaveGame_Implementation(int32 gameVersion, int32 engineVersion) override;
	// End IFGSaveInterface

	// Begin IFINNetworkCustomType
	virtual FString GetCustomTypeName_Implementation() const override { return TEXT("Computer"); }
	// End IFINNetworkCustomType

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
	EComputerState GetState();

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
    void WriteSerialInput(const FString& str);
	
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	FString GetSerialOutput();

	UFUNCTION()
	void HandleSignal(const FFINDynamicStructHolder& signal, const FFINNetworkTrace& sender);

	UFUNCTION()
	void OnDriveUpdate(bool bOldLocked, AFINFileSystemState* drive);
};
