#pragma once

#include "CoreMinimal.h"
#include "FIVSNode.h"
#include "FIVSGraph.generated.h"

class UFIVSNode;

UENUM()
enum EFIVSNodeChange {
	FIVS_Node_Added,
	FIVS_Node_Removed,
};

/**
 * Notifies if the node list of the graph has changed.
 * Param1: type of change (0 = node added, 1 = node removed)
 * Param2: the changed node
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINScriptGraphNodeChanged, EFIVSNodeChange, Type, UFIVSNode*, Node);

UCLASS(BlueprintType)
class UFIVSGraph : public UObject {
	GENERATED_BODY()
private:
	UPROPERTY()
	TArray<UFIVSNode*> Nodes;

	TMap<UFIVSNode*, FDelegateHandle> NodeDelegateHandles;

public:
	UPROPERTY(BlueprintAssignable)
	FFINScriptGraphNodeChanged OnNodeChangedEvent;
	TMulticastDelegate<void(EFIVSNodeChange, UFIVSNode*)> OnNodeChanged;
	
	/**
	 * Adds a new node to the graph.
	 *
	 * @param[in]	Node	the new node you want to add to the graph.
	 * @return	the index of the node, -1 if not able to add.
	 */
	UFUNCTION(BlueprintCallable)
	int AddNode(UFIVSNode* Node);

	/**
	 * Removes the given node from the graph.
	 *
	 * @param[in]	Node	the node you want to remove
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveNode(UFIVSNode* Node);

	/**
	 * Returns all nodes of the graph.
	 */
	UFUNCTION(BlueprintCallable)
	const TArray<UFIVSNode*>& GetNodes() const;

	UFUNCTION(BlueprintCallable)
	void RemoveAllNodes();
	
private:
	void OnPinChanged(EFIVSNodePinChange, UFIVSPin*);
};
