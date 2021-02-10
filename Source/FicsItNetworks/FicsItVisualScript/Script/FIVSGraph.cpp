#include "FIVSGraph.h"

int UFIVSGraph::AddNode(UFIVSNode* Node) {
	int idx = Nodes.AddUnique(Node);
	if (idx >= 0) {
		OnNodeChanged.Broadcast(0, Node);
	}
	return idx;
}

void UFIVSGraph::RemoveNode(UFIVSNode* Node) {
	if (Nodes.Contains(Node)) {
		Node->RemoveAllConnections();
		OnNodeChanged.Broadcast(1, Node);
		Nodes.Remove(Node);
	}
}

const TArray<UFIVSNode*>& UFIVSGraph::GetNodes() const {
	return Nodes;
}
