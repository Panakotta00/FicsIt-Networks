#pragma once

#include "CoreMinimal.h"
#include "Network/FINDynamicStructHolder.h"
#include "FIVSScript.generated.h"

struct FFIVSNodeStatement;

USTRUCT()
struct FFIVSScript {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FGuid StartNode;

	UPROPERTY(SaveGame)
	TMap<FGuid, FFINDynamicStructHolder> Nodes;

	UPROPERTY(SaveGame)
	TMap<FGuid, FGuid> PinConnections;

	UPROPERTY(SaveGame)
	TMap<FGuid, FFINAnyNetworkValue> PinLiterals;

	UPROPERTY()
	TMap<FGuid, FGuid> PinToNode;

	TOptional<TFINDynamicStruct<const FFIVSNodeStatement>> FindNode(FGuid NodeId) const {
		const FFINDynamicStructHolder* holder = Nodes.Find(NodeId);
		if (!holder) return {};
		return TFINDynamicStruct<const FFIVSNodeStatement>(*holder);
	}
};
