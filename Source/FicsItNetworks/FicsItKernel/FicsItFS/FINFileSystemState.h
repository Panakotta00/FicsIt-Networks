#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FGInventoryComponent.h"
#include "FicsItKernel/FicsItFS/FileSystem.h"
#include "FINFileSystemState.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINFileSystemState : public AActor, public IFGSaveInterface {
	GENERATED_BODY()

private:
	FileSystem::SRef<FileSystem::Device> Device;
	
public:
	UPROPERTY(SaveGame)
	FGuid ID;

	UPROPERTY(SaveGame)
	bool IdCreated = false;

	UPROPERTY(SaveGame, EditDefaultsOnly)
	int32 Capacity = 0;

	AFINFileSystemState();
	~AFINFileSystemState();

	// Begin UObject
	virtual void Serialize(FArchive& Ar) override;
	// End UObject
	
	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
	FileSystem::SRef<FileSystem::Device> GetDevice();

	/**
	 * Creates a new item state object wich holds information and functions about a save game saved filesystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "FileSystem")
	static AFINFileSystemState* CreateState(UObject* WorldContextObject, int32 inCapacity, UFGInventoryComponent* inInventory, int32 inSlot);
};