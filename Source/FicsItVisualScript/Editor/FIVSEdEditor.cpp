#include "FIVSEdEditor.h"

#include "FIVSEdGraphViewer.h"
#include "Slate/Private/Framework/Docking/SDockingArea.h"

void UFIVSEdEditor::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);

	Viewer.Reset();
}

TSharedRef<SWidget> UFIVSEdEditor::RebuildWidget() {
	return SNew(SOverlay)
	+SOverlay::Slot()[
		SNew(SImage)
		.Image(&BackgroundBrush)
	]
	+SOverlay::Slot()[
		SNew(SGridPanel)
		.FillColumn(1, 1)
		.FillRow(0, 1)
		+SGridPanel::Slot(0, 0)
		.VAlign(VAlign_Top)[
			SNew(SVerticalBox)
			//+SVerticalBox::Slot()[
			//	SNew(STextBlock)
			//	.Text(FText::FromString(TEXT("Global Variables")))
			//]
			//+SVerticalBox::Slot()[
			//	
			//];
			+SVerticalBox::Slot()[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Details")))
			]
			+SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Fill)[
				SAssignNew(SelectionDetailsContainer, SBox)
				.MinDesiredWidth(300)
				.MinDesiredHeight(100)
			]
		]
		+SGridPanel::Slot(1, 0)[
			SAssignNew(Viewer, SFIVSEdGraphViewer)
			.Style(&Style)
			.Graph(Graph)
			.OnSelectionChanged_Lambda([this](UFIVSNode* Node, bool bSelected) {
				UpdateSelection();
			})
		]
	];
}

void UFIVSEdEditor::SetGraph(UFIVSGraph* InGraph) {
	Graph = InGraph;
	if (Viewer.IsValid()) Viewer->SetGraph(Graph);
}
UFIVSGraph* UFIVSEdEditor::GetGraph() const {
	return Graph;
}

void UFIVSEdEditor::SetContext(TScriptInterface<IFIVSScriptContext_Interface> InContext) {
	Context = InContext;
}

void UFIVSEdEditor::UpdateSelection() {
	TSharedPtr<SWidget> DetailsWidget;
	if (Viewer->SelectionManager.GetSelection().Num() == 1) {
		DetailsWidget = Viewer->SelectionManager.GetSelection()[0]->CreateDetailsWidget(Context);
	}
	if (DetailsWidget.IsValid()) SelectionDetailsContainer->SetContent(DetailsWidget.ToSharedRef());
	else SelectionDetailsContainer->SetContent(SNew(SSpacer));
}
