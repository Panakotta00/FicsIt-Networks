#include "Editor/FIVSEdEditor.h"

#include "Editor/FIVSEdGraphViewer.h"
#include "Script/FIVSGraph.h"
#include "Script/Library/FIVSMathLib.h"
#include "Script/Library/FIVSNode_Branch.h"
#include "Script/Library/FIVSNode_OnTick.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SSpacer.h"

void UFIVSEdEditor::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);

	Viewer.Reset();
}

TSharedRef<SWidget> UFIVSEdEditor::RebuildWidget() {
	if (!IsValid(Graph)) {
		Graph = NewObject<UFIVSGraph>();

		UFIVSNode_OnTick* OnTick = NewObject<UFIVSNode_OnTick>(Graph);
		OnTick->Pos = FVector2D(300.0, 300.0);
		Graph->AddNode(OnTick);
		UFIVSRerouteNode* Reroute = NewObject<UFIVSRerouteNode>(Graph);
		Reroute->Pos = FVector2D(500.0, 350.0);
		Graph->AddNode(Reroute);
		UFIVSNode_Branch* Branch = NewObject<UFIVSNode_Branch>(Graph);
		Branch->Pos = FVector2D(800.0, 300.0);
		Graph->AddNode(Branch);
		UFIVSNode_LuaGeneric* OpAdd = NewObject<UFIVSNode_LuaGeneric>(Graph);
		OpAdd->Pos = FVector2D(500.0, 600.0);
		OpAdd->SetFunction(UFIVSMathLib::StaticClass()->FindFunctionByName(TEXT("FIVSFunc_Float_Addition")));
		Graph->AddNode(OpAdd);

		OnTick->GetNodePins()[0]->AddConnection(Reroute->GetNodePins()[0]);
		Reroute->GetNodePins()[0]->AddConnection(Branch->GetNodePins()[0]);
	}

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
			.Context(this)
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
	MyWidget.Reset();
	InvalidateLayoutAndVolatility();
}

void UFIVSEdEditor::UpdateSelection() {
	TSharedPtr<SWidget> DetailsWidget;
	if (Viewer->SelectionManager.GetSelection().Num() == 1) {
		DetailsWidget = Viewer->SelectionManager.GetSelection()[0]->CreateDetailsWidget(Context);
	}
	if (DetailsWidget.IsValid()) SelectionDetailsContainer->SetContent(DetailsWidget.ToSharedRef());
	else SelectionDetailsContainer->SetContent(SNew(SSpacer));
}
