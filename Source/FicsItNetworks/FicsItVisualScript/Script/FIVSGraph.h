#pragma once

#include "CoreMinimal.h"

#include "FIVSNode.h"
#include "FIVSGraph.generated.h"

/**
 * Notifies if the node list of the graph has changed.
 * Param1: type of change (0 = node added, 1 = node removed)
 * Param2: the changed node
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FFINScriptGraphNodeChanged, int, UFIVSNode*);

UCLASS()
class UFIVSGraph : public UObject {
	GENERATED_BODY()
private:
	UPROPERTY()
	TArray<UFIVSNode*> Nodes;

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
};
