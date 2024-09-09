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
DECLARE_MULTICAST_DELEGATE_TwoParams(FFINScriptGraphNodeChanged, EFIVSNodeChange, UFIVSNode*);

UCLASS(BlueprintType)
class UFIVSGraph : public UObject {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TArray<UFIVSNode*> Nodes;

	TMap<UFIVSNode*, FDelegateHandle> NodeDelegateHandles;

public:
	FFINScriptGraphNodeChanged OnNodeChanged;
	
	/**
	 * Adds a new node to the graph.
	 *
	 * @param[in]	Node	the new node you want to add to the graph.
	 * @return	the index of the node, -1 if not able to add.
	 */
	int AddNode(UFIVSNode* Node);

	/**
	 * Removes the given node from the graph.
	 *
	 * @param[in]	Node	the node you want to remove
	 */
	void RemoveNode(UFIVSNode* Node);

	/**
	 * Returns all nodes of the graph.
	 */
	const TArray<UFIVSNode*>& GetNodes() const;
	
private:
	void OnPinChanged(EFIVSNodePinChange, UFIVSPin*);
};
