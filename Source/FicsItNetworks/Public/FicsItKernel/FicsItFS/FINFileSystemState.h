#pragma once

#include "FicsItKernel/FicsItFS/FileSystem.h"
#include "FGInventoryComponent.h"
#include "GameFramework/Actor.h"
#include "Utils/FINLabelContainerInterface.h"
#include "FINFileSystemState.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINFileSystemState : public AActor, public IFGSaveInterface, public IFINLabelContainerInterface {
	GENERATED_BODY()

private:
	CodersFileSystem::SRef<CodersFileSystem::Device> Device;

	bool bUseOldSerialization = false;
	bool bUsePreBinarySupportSerialization = true;
	
public:
	void SerializePath(CodersFileSystem::SRef<CodersFileSystem::Device> SerializeDevice, FStructuredArchive::FRecord Record, CodersFileSystem::Path Path, FString Name, int& KeepDisk);

	UPROPERTY(SaveGame, Replicated)
	FGuid ID;

	UPROPERTY(SaveGame, Replicated)
	bool IdCreated = false;

	UPROPERTY(SaveGame, Replicated, EditDefaultsOnly)
	int32 Capacity = 0;

	UPROPERTY(SaveGame, Replicated)
	FString Label;

	UPROPERTY(Replicated)
	float Usage = 0.0f;

	FTimerHandle UsageUpdateHandler;
	
	AFINFileSystemState();
	~AFINFileSystemState();

	// Begin UObject
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	// End UObject
	
	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin IFGSaveInterface
	virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFINLabelContainerInterface
	virtual FString GetLabel_Implementation() override;
	virtual void SetLabel_Implementation(const FString& InLabel) override;
	// End IFINLabelContainerInterface
	
	CodersFileSystem::SRef<CodersFileSystem::Device> GetDevice(bool bInForceUpdate = false, bool bInForeCreate = false);

	/**
	 * Creates a new item state object wich holds information and functions about a save game saved filesystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "FileSystem")
	static AFINFileSystemState* CreateState(UObject* WorldContextObject, int32 inCapacity, UFGInventoryComponent* inInventory, int32 inSlot);

	UFUNCTION()
	void UpdateUsage();

	void Serialize_DEPRECATED(FArchive& Ar);
};