#pragma once

#include "CoreMinimal.h"
#include "FGRailroadSignalBlock.h"
#include "FIRRailroadSignalBlock.generated.h"

USTRUCT()
struct FICSITREFLECTION_API FFIRRailroadSignalBlock {
	GENERATED_BODY()
	
	TWeakPtr<FFGRailroadSignalBlock> Block;
	
	FFIRRailroadSignalBlock() = default;
	FFIRRailroadSignalBlock(TWeakPtr<FFGRailroadSignalBlock> Block) : Block(Block) {}
};
