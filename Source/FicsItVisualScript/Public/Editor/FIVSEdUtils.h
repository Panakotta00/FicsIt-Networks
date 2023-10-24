#pragma once

#include "CoreMinimal.h"
#include "Components/NativeWidgetHost.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Script/FIVSGraph.h"
#include "FIVSEdGraphViewer.h"
#include "FIVSEdUtils.generated.h"

UCLASS()
class UFIVSEdUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSGraph* Graph = nullptr;
	
public:
	UFUNCTION(BlueprintCallable)
	static void TestFicsItVisualScript(UNativeWidgetHost* Widget) {
		UFIVSEdUtils* Utility = Cast<UFIVSEdUtils>(UFIVSEdUtils::StaticClass()->GetDefaultObject());
		
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
};