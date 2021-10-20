#pragma once

#include "CoreMinimal.h"
#include "FIVSNodeSerialization.generated.h"

class UFIVSGraph;
class UFIVSNode;

USTRUCT()
struct FFIVSNodeProperties {
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<FString, FString> Properties;
};

USTRUCT()
struct FFIVSSerializedPin {
	GENERATED_BODY()
	
	UPROPERTY()
	FString PinName;

	TSharedPtr<FJsonValue> PinLiteralValue;
};

USTRUCT()
struct FFIVSSerializedNode {
	GENERATED_BODY()
	
	UPROPERTY()
	TSubclassOf<UFIVSNode> NodeType;

	UPROPERTY()
	FVector2D NodePos;

	UPROPERTY()
	int NodeID;

	UPROPERTY()
	FFIVSNodeProperties Properties;

	UPROPERTY()
	TArray<FFIVSSerializedPin> Pins;
};

USTRUCT()
struct FFIVSSerializedPinReference {
	GENERATED_BODY()

	UPROPERTY()
	int NodeID;

	UPROPERTY()
	FString PinName;
};

USTRUCT()
struct FFIVSSerializedPinConnection {
	GENERATED_BODY()

	UPROPERTY()
	FFIVSSerializedPinReference Pin1;

	UPROPERTY()
	FFIVSSerializedPinReference Pin2;
};

USTRUCT()
struct FFIVSSerializedGraph {
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FFIVSSerializedNode> Nodes;

	UPROPERTY()
	TArray<FFIVSSerializedPinConnection> PinConnections;
};

UCLASS()
class UFIVSSerailizationUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FString FIVS_SerializePartial(TArray<UFIVSNode*> InNodes, bool bZeroOffset);

	UFUNCTION(BlueprintCallable)
	static FString FIVS_SerailizeGraph(UFIVSGraph* Graph);

	UFUNCTION(BlueprintCallable)
	static void FIVS_DeserializeGraph(UFIVSGraph* Graph, FString InStr, FVector2D InOffset = FVector2D::ZeroVector);
};
