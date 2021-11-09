#include "FIVSGraph.h"

#include "FIVSNode.h"

int UFIVSGraph::AddNode(UFIVSNode* Node) {
	int idx;
	if (!Nodes.Find(Node, idx)) {
		idx = Nodes.Add(Node);
		OnNodeChanged.Broadcast(0, Node);
		NodeDelegateHandles.Add(Node, Node->OnPinChanged.AddUObject(this, &UFIVSGraph::OnPinChanged));
	}
	return idx;
}

void UFIVSGraph::RemoveNode(UFIVSNode* Node) {
	if (Nodes.Contains(Node)) {
		Node->RemoveAllConnections();
		FDelegateHandle Handle;
		NodeDelegateHandles.RemoveAndCopyValue(Node, Handle);
		Node->OnPinChanged.Remove(Handle);
		OnNodeChanged.Broadcast(1, Node);
		Nodes.Remove(Node);
	}
}

const TArray<UFIVSNode*>& UFIVSGraph::GetNodes() const {
	return Nodes;
}

void UFIVSGraph::OnPinChanged(EFIVSNodePinChange Change, UFIVSPin* Pin) {
	
}
