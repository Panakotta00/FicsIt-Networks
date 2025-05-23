﻿#pragma once

#include "CoreMinimal.h"
#include "Device.h"
#include "FGSubsystem.h"
#include "FINFileSystemSubsystem.generated.h"

struct FFINItemStateFileSystem;

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINFileSystemSubsystem : public AFGSubsystem {
	GENERATED_BODY()
public:
	/**
	 * Returns a CFS Device for the given FileSystem-State Struct.
	 * Tries to hit cache, if missed, creates a Device instead.
	 *
	 * This function has to work on static because the devices are used for Serialization of the FileSystem.
	 */
	static TSharedPtr<CodersFileSystem::Device> GetDevice(const FGuid& FileSystemID, bool bInForceUpdate = false, bool bInForeCreate = false);

	UFUNCTION(BlueprintCallable, Category = "Computer", meta = (WorldContext = "WorldContext"))
	static AFINFileSystemSubsystem* GetFileSystemSubsystem(UObject* WorldContext);

	/**
	 * Creates a new item state object wich holds information and functions about a save game saved filesystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "FileSystem")
	static FGuid CreateState(int32 inCapacity, class UFGInventoryComponent* inInventory, int32 inSlot);
	static FGuid CreateState(int32 inCapacity, FInventoryItem& inItem);

	/**
	 * Returns the Usage of a File System (0.0 - 1.0)
	 */
	float GetUsage(FGuid Guid) {
		// TODO: 1.0: FS Usage
		return 0.0;
	}

private:
	static FCriticalSection DevicesMutex;
	static TMap<FGuid, TSharedRef<CodersFileSystem::Device>> Devices;

	// TODO: FileSystem Usage
	FCriticalSection UsageMutex;
	TMap<FGuid, double> Usage;

	FTimerHandle UsageUpdateHandler;
};
