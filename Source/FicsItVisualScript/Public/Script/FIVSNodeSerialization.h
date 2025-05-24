#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FIVSNodeSerialization.generated.h"

UCLASS()
class UFIVSSerailizationUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FString FIVS_SerializePartial(TArray<UFIVSNode*> InNodes, bool bZeroOffset);

	UFUNCTION(BlueprintCallable)
	static FString FIVS_SerailizeGraph(UFIVSGraph* Graph);

	UFUNCTION(BlueprintCallable)
	static TArray<UFIVSNode*> FIVS_DeserializeGraph(UFIVSGraph* Graph, FString InStr, bool bCreateNewGuids);

	UFUNCTION(BlueprintCallable)
	static void FIVS_AdjustNodesOffset(const TArray<UFIVSNode*>& InNodes, FVector2D Offset, bool bRelativeToCenter);
};
