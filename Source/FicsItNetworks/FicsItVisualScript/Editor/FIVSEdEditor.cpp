#include "FIVSEdEditor.h"

#include "FIVSEdGraphViewer.h"

TSharedRef<SWidget> UFIVSEdEditor::RebuildWidget() {
	return SAssignNew(Viewer, SFIVSEdGraphViewer)
	.Graph(Graph);
}

void UFIVSEdEditor::SetGraph(UFIVSGraph* InGraph) {
	Graph = InGraph;
	if (MyWidget.IsValid()) StaticCastSharedPtr<SFIVSEdGraphViewer>(MyWidget.Pin())->SetGraph(Graph);
}

UFIVSGraph* UFIVSEdEditor::GetGraph() const {
	return Graph;
}
