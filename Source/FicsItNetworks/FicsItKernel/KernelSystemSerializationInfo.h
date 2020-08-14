#pragma once

#include "CoreMinimal.h"
#include "FicsItFS/FileSystemSerializationInfo.h"
#include "Processor/ProcessorStateStorage.h"
#include "KernelSystemSerializationInfo.generated.h"

USTRUCT()
struct FKernelSystemSerializationInfo {
	GENERATED_BODY()

	UPROPERTY()
	int32 systemState;

	UPROPERTY()
	UProcessorStateStorage* processorState;

	UPROPERTY()
	FFileSystemSerializationInfo fileSystemState;

	UPROPERTY()
	FString devDeviceMountPoint;

	UPROPERTY()
	FString crash;

	UPROPERTY()
	int64 MillisSinceLastReset = 0;

	bool bPreSerialized = false;
};
