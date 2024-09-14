#include "Kernel/FIVSRuntimeContext.h"

#include "Script/FIVSScriptNode.h"

void FFIVSRuntimeContext::NextStep() {
	auto entryOpt = PopStackEntry();
	if (!entryOpt) return;
	FFIVSStackEntry entry = *entryOpt;

	switch (entry.Type) {
		case FIVS_Stack_PreNode: {
			if (entry.bExec) {
				ExecutedVolatileNodes = ExecutedNodes;
			} else if (ExecutedVolatileNodes.Contains(entry.Node)) {
				break;
			}
			ExecutedVolatileNodes.Add(entry.Node);

			PushStackEntry(FFIVSStackEntry(FIVS_Stack_Node, entry.Node, entry.Pin, entry.bExec));

			TOptional<TFINDynamicStruct<const FFIVSNodeStatement>> node = GetScript()->FindNode(entry.Node);
			if (!node.IsSet()) {
				break;
			}

			if (node.GetValue()->IsVolatile() == false) {
				ExecutedNodes.Add(entry.Node);
			}

			node.GetValue()->PreExecPin(*this, entry.Pin);

			break;
		} case FIVS_Stack_Node: {
			TFINDynamicStruct<const FFIVSNodeStatement> node = GetScript()->FindNode(entry.Node).GetValue();

			node->ExecPin(*this, entry.Pin);

			break;
		} default: ;
	}
}
