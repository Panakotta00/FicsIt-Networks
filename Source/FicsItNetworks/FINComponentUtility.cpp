#include "FINComponentUtility.h"

#include "FGPlayerController.h"
#include "FicsItVisualScript/Editor/FIVSEdGraphViewer.h"
#include "Network/FINNetworkAdapter.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

UFINNetworkConnectionComponent* UFINComponentUtility::GetNetworkConnectorFromHit(FHitResult hit) {
	if (!hit.bBlockingHit) return nullptr;

	UFINNetworkConnectionComponent* connector = nullptr;
	FVector pos;

	if (!hit.Actor.IsValid()) return nullptr;
	
	TArray<UActorComponent*> connectors = hit.Actor->GetComponentsByClass(UFINNetworkConnectionComponent::StaticClass());

	for (UActorComponent* con : connectors) {
		if (!Cast<USceneComponent>(con)) continue;

		FVector npos = Cast<USceneComponent>(con)->GetComponentLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = Cast<UFINNetworkConnectionComponent>(con);
		}
	}

	if (connector) return connector;
	
	TArray<UActorComponent*> adapters = hit.Actor->GetComponentsByClass(UFINNetworkAdapterReference::StaticClass());
	
	for (UActorComponent* adapterref : adapters) {
		if (!adapterref || !static_cast<UFINNetworkAdapterReference*>(adapterref)->Ref) continue;

		FVector npos = static_cast<UFINNetworkAdapterReference*>(adapterref)->Ref->GetActorLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = static_cast<UFINNetworkAdapterReference*>(adapterref)->Ref->Connector;
		}
	}

	return connector;
}

void UFINComponentUtility::ClipboardCopy(FString str) {
	FWindowsPlatformApplicationMisc::ClipboardCopy(*str);
}

void UFINComponentUtility::TestFicsItVisualScript(UNativeWidgetHost* Widget) {
	UFINComponentUtility* Utility = Cast<UFINComponentUtility>(UFINComponentUtility::StaticClass()->GetDefaultObject());
	
	UFIVSGraph* Graph = Utility->Graph;
	if (Graph) {
		TArray<UFIVSNode*> Nodes = Graph->GetNodes();
		for (UFIVSNode* Node : Nodes) Graph->RemoveNode(Node);
	} else {
		Utility->Graph = Graph = NewObject<UFIVSGraph>();
	}
	/*UFIVSGenericFuncNode* Node = NewObject<UFIVSGenericFuncNode>();
	Node->Name = "Node1";
	Node->AddPin(UFIVSGenericPin::Create(FIN_BOOL, FIVS_PIN_DATA_INPUT, "In1"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_CLASS, FIVS_PIN_DATA_INPUT, "In2"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_FLOAT, FIVS_PIN_DATA_INPUT, "In3"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_INT, FIVS_PIN_DATA_INPUT, "In4"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_OBJ, FIVS_PIN_DATA_OUTPUT, "Out1"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_STR, FIVS_PIN_DATA_OUTPUT, "Out2"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_STRUCT, FIVS_PIN_DATA_OUTPUT, "Out3"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_TRACE, FIVS_PIN_DATA_OUTPUT, "Out4"));
	Graph->AddNode(Node);
	Node = NewObject<UFIVSGenericFuncNode>();
	Node->Name = "Node2";
	Node->Pos = FVector2D(200,0);
	Node->AddPin(UFIVSGenericPin::Create(FIN_BOOL, FIVS_PIN_DATA_OUTPUT, "Out1"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_CLASS, FIVS_PIN_DATA_INPUT, "In1"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_FLOAT, FIVS_PIN_DATA_OUTPUT, "Out2"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_INT, FIVS_PIN_DATA_INPUT, "In2"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_OBJ, FIVS_PIN_DATA_OUTPUT, "Out3"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_STR, FIVS_PIN_DATA_INPUT, "In3"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_STRUCT, FIVS_PIN_DATA_OUTPUT, "Out4"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_TRACE, FIVS_PIN_DATA_INPUT, "In4"));
	Graph->AddNode(Node);
	Node = NewObject<UFIVSGenericFuncNode>();
	Node->Name = "Node3";
	Node->Pos = FVector2D(0,200);
	Node->AddPin(UFIVSGenericPin::Create(FIN_BOOL, FIVS_PIN_DATA_OUTPUT, "Out1"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_CLASS, FIVS_PIN_DATA_OUTPUT, "Out2"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_FLOAT, FIVS_PIN_DATA_OUTPUT, "Out3"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_INT, FIVS_PIN_DATA_OUTPUT, "Out4"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_OBJ, FIVS_PIN_DATA_INPUT, "In1"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_STR, FIVS_PIN_DATA_INPUT, "IN2"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_STRUCT, FIVS_PIN_DATA_INPUT, "In3"));
	Node->AddPin(UFIVSGenericPin::Create(FIN_TRACE, FIVS_PIN_DATA_INPUT, "In4"));
	Graph->AddNode(Node);
	UFIVSRerouteNode* Reroute = NewObject<UFIVSRerouteNode>();
	Reroute->Pos = FVector2D(200, 200);
	Graph->AddNode(Reroute);
	Reroute = NewObject<UFIVSRerouteNode>();
	Reroute->Pos = FVector2D(300, 200);
	Graph->AddNode(Reroute);*/
	if (Widget) Widget->SetContent(SNew(SFIVSEdGraphViewer).Graph(Graph));
}
