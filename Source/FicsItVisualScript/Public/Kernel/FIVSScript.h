#pragma once

#include "CoreMinimal.h"
#include "FIRInstancedStruct.h"
#include "FIRAnyValue.h"
#include "FIVSScript.generated.h"

struct FFIVSNodeStatement;

USTRUCT()
struct FFIVSScript {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FGuid StartNode;

	UPROPERTY(SaveGame)
	TMap<FGuid, FFIRInstancedStruct> Nodes;

	UPROPERTY(SaveGame)
	TMap<FGuid, FGuid> PinConnections;

	UPROPERTY(SaveGame)
	TMap<FGuid, FFIRAnyValue> PinLiterals;

	UPROPERTY()
	TMap<FGuid, FGuid> PinToNode;

	TOptional<TFIRInstancedStruct<const FFIVSNodeStatement>> FindNode(FGuid NodeId) const {
		const FFIRInstancedStruct* holder = Nodes.Find(NodeId);
		if (!holder) return {};
		return TFIRInstancedStruct<const FFIVSNodeStatement>(*holder);
	}
};
