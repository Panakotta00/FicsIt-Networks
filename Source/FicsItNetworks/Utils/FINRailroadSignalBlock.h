#pragma once

#include "CoreMinimal.h"
#include "FGRailroadSignalBlock.h"
#include "FINRailroadSignalBlock.generated.h"

USTRUCT()
struct FICSITNETWORKS_API FFINRailroadSignalBlock {
	GENERATED_BODY()
	
	TWeakPtr<FFGRailroadSignalBlock> Block;
	
	FFINRailroadSignalBlock() = default;
	FFINRailroadSignalBlock(TWeakPtr<FFGRailroadSignalBlock> Block) : Block(Block) {}
};
