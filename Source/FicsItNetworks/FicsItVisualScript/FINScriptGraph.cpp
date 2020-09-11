#include "FINScriptGraph.h"

int UFINScriptGraph::AddNode(UFINScriptNode* Node) {
	int idx = Nodes.AddUnique(Node);
	if (idx >= 0) {
		OnNodeChanged.Broadcast(0, Node);
	}
	return idx;
}

void UFINScriptGraph::RemoveNode(UFINScriptNode* Node) {
	if (Nodes.Contains(Node)) {
		Node->RemoveAllConnections();
		OnNodeChanged.Broadcast(1, Node);
		Nodes.Remove(Node);
	}
}

const TArray<UFINScriptNode*>& UFINScriptGraph::GetNodes() const {
	return Nodes;
}
