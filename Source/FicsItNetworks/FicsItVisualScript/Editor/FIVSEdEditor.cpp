#include "FIVSEdEditor.h"

#include "FIVSEdGraphViewer.h"

void UFIVSEdEditor::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);

	Viewer.Reset();
}

TSharedRef<SWidget> UFIVSEdEditor::RebuildWidget() {
	return SAssignNew(Viewer, SFIVSEdGraphViewer)
	.Style(&Style)
	.Graph(Graph);
}

void UFIVSEdEditor::SetGraph(UFIVSGraph* InGraph) {
	Graph = InGraph;
	if (Viewer.IsValid()) Viewer->SetGraph(Graph);
}

UFIVSGraph* UFIVSEdEditor::GetGraph() const {
	return Graph;
}
