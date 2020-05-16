#pragma once

#include "CoreMinimal.h"
#include "Processor/ProcessorStateStorage.h"
#include "KernelSystemSerializationInfo.generated.h"

USTRUCT()
struct FKernelSystemSerializationInfo {
	GENERATED_BODY()

	UPROPERTY()
	int32 systemState;

	UPROPERTY()
	UProcessorStateStorage* processorState;
};
