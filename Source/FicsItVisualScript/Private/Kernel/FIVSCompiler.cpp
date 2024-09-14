#include "Kernel/FIVSCompiler.h"

#include "Kernel/FIVSScript.h"
#include "Script/FIVSGraph.h"
#include "Script/FIVSScriptNode.h"

TMap<UFIVSScriptNode*, FFIVSScript> FFIVSCompiler::CompileGraph(UFIVSGraph* Graph) {
	TMap<UFIVSScriptNode*, FFIVSScript> scripts;

	for (UFIVSNode* node : Graph->GetNodes()) {
		UFIVSScriptNode* scriptNode = Cast<UFIVSScriptNode>(node);
		if (!IsValid(scriptNode) || !scriptNode->IsStartNode()) continue;

		FFIVSScript script = CompileScript(scriptNode);
		scripts.Add(scriptNode, script);
	}

	return scripts;
}

FFIVSScript FFIVSCompiler::CompileScript(UFIVSScriptNode* Node) {
	FFIVSScript script;

	script.StartNode = Node->NodeId;

	TArray<UFIVSScriptNode*> toDo = {Node};
	while (!toDo.IsEmpty()) {
		UFIVSScriptNode* node = toDo.Pop();
		if (!IsValid(node) || script.Nodes.Contains(node->NodeId)) continue;
		script.Nodes.Add(node->NodeId, node->CreateNodeStatement());
		for (UFIVSPin* pin : node->GetNodePins()) {
			script.PinToNode.Add(pin->PinId, node->NodeId);
			UFIVSPin* connected = pin->FindConnected();
			if (connected) {
				script.PinConnections.Add(pin->PinId, connected->PinId);
				toDo.Push(Cast<UFIVSScriptNode>(connected->ParentNode));
			} else {
				script.PinLiterals.Add(pin->PinId, pin->GetLiteral());
			}
		}
	}

	return script;
}
