#include "Script/FIVSNode.h"

#include "Editor/FIVSEdNodeViewer.h"
#include "Script/FIVSGraph.h"

void UFIVSNode::RemoveAllConnections() {
	for (UFIVSPin* Pin : GetNodePins()) {
		Pin->RemoveAllConnections();
	}
}

UFIVSGraph* UFIVSNode::GetOuterGraph() const {
	return Cast<UFIVSGraph>(GetOuter());
}

TSharedRef<SFIVSEdNodeViewer> UFIVSNode::CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style) {
	return SNew(SFIVSEdFunctionNodeViewer, GraphViewer, this)
		.Style(Style);
}

UFIVSRerouteNode::UFIVSRerouteNode() {
	Pin = CreateDefaultSubobject<UFIVSWildcardPin>("Pin");
	Cast<UFIVSWildcardPin>(Pin)->DisplayName = FText();
	Pin->ParentNode = this;
}

TArray<UFIVSPin*> UFIVSRerouteNode::GetNodePins() const {
	return {Pin};
}

void UFIVSRerouteNode::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	Actions.Add({
		UFIVSRerouteNode::StaticClass(),
		FText::FromString("Create Reroute node"),
		FText::FromString(""),
		FText::FromString("Create Reroute Node"),
		{
			{FIVS_PIN_DATA_INPUT | FIVS_PIN_EXEC_OUTPUT, FFIVSPinDataType(FIN_ANY)}
		}
	});
}

TSharedRef<SFIVSEdNodeViewer> UFIVSRerouteNode::CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style) {
	return SNew(SFIVSEdRerouteNodeViewer, GraphViewer, this)
		.Style(Style);
}
