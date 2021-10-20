#include "FIVSNodeSerialization.h"

#include "FicsItNetworks/FicsItNetworksModule.h"
#include "FIVSGraph.h"
#include "FIVSNode.h"
#include "JsonObjectConverter.h"

TSharedPtr<FJsonValue> FIVS_GetPinLiteralAsJson(UFIVSPin* InPin) {
	if (InPin->GetPinType() & FIVS_PIN_EXEC) return nullptr;
	switch (InPin->GetPinDataType().GetType()) {
	case FIN_NIL:
		return nullptr;
	case FIN_BOOL:
		return MakeShared<FJsonValueBoolean>(InPin->GetLiteral().GetBool());
	case FIN_INT:
		return MakeShared<FJsonValueNumber>(InPin->GetLiteral().GetInt());
	case FIN_FLOAT:
		return MakeShared<FJsonValueNumber>(InPin->GetLiteral().GetFloat());
	case FIN_STR:
		return MakeShared<FJsonValueString>(InPin->GetLiteral().GetString());
	case FIN_OBJ:
		return MakeShared<FJsonValueString>(InPin->GetLiteral().GetObj().Get()->GetPathName());
	case FIN_CLASS:
		return MakeShared<FJsonValueString>(InPin->GetLiteral().GetClass()->GetPathName());
	case FIN_TRACE:
		return nullptr; // TODO: Trace to JSON
	case FIN_STRUCT:
		return nullptr; // TODO: Maybe add Struct serialization here, but maybe this is broken up before hand and pins can't have struct literals, instead the pins may have to be split up into individual pins which can then have a literal
	case FIN_ARRAY:
		return nullptr; // TODO: Maybe add Array serialization, but pins would actually have to support it
	case FIN_ANY:
		return nullptr; // TODO: Pins with FINAny... I have no clue, is that even possible?
	default: ;
	}
	return nullptr;
}

void FIVS_SetPinLiteralFromJson(UFIVSPin* InPin, TSharedPtr<FJsonValue> InValue) {
	if (InPin->GetPinType() & FIVS_PIN_EXEC) return;
	switch (InPin->GetPinDataType().GetType()) {
	case FIN_NIL:
		return;
	case FIN_BOOL:
		return InPin->SetLiteral(InValue->AsBool());
	case FIN_INT:
		return InPin->SetLiteral((FINInt)InValue->AsNumber());
	case FIN_FLOAT:
		return InPin->SetLiteral(InValue->AsNumber());
	case FIN_STR:
		return InPin->SetLiteral(InValue->AsString());
	case FIN_OBJ:
		return InPin->SetLiteral((FINObj)FSoftObjectPath(InValue->AsString()).TryLoad());
	case FIN_CLASS:
		return InPin->SetLiteral(Cast<UClass>(FSoftObjectPath(InValue->AsString()).TryLoad()));
	case FIN_TRACE:
		// TODO: Trace to JSON
	case FIN_STRUCT:
		// TODO: Maybe add Struct serialization here, but maybe this is broken up before hand and pins can't have struct literals, instead the pins may have to be split up into individual pins which can then have a literal
	case FIN_ARRAY:
		// TODO: Maybe add Array serialization, but pins would actually have to support it
	case FIN_ANY:
		// TODO: Pins with FINAny... I have no clue, is that even possible?
	default: ;
	}
}

FString UFIVSSerailizationUtils::FIVS_SerializePartial(TArray<UFIVSNode*> InNodes, bool bZeroOffset) {
	FFIVSSerializedGraph Graph;
	TMap<UFIVSNode*, int> SerializedNodes;
	FVector2D LargestOffset = FVector2D(TNumericLimits<float>::Max(), TNumericLimits<float>::Max());
	for (UFIVSNode* Node : InNodes) {
		LargestOffset.X = FMath::Min(LargestOffset.X, Node->Pos.X);
		LargestOffset.Y = FMath::Min(LargestOffset.Y, Node->Pos.Y);
		FFIVSSerializedNode SerializedNode;
		SerializedNode.NodeType = Node->GetClass();
		SerializedNode.NodePos = Node->Pos;
		Node->SerializeNodeProperties(SerializedNode.Properties);
		SerializedNode.NodeID = Graph.Nodes.Num();
		for (UFIVSPin* Pin : Node->GetNodePins()) {
			FFIVSSerializedPin SerializedPin;
			SerializedPin.PinName = Pin->GetName().ToString();
			SerializedPin.PinLiteralValue = FIVS_GetPinLiteralAsJson(Pin);
			for (UFIVSPin* Connected : Pin->GetConnections()) {
				int* ConnectedNodeRef = SerializedNodes.Find(Connected->ParentNode);
				if (ConnectedNodeRef) {
					FFIVSSerializedPinConnection Connection;
					Connection.Pin1.NodeID = *ConnectedNodeRef;
					Connection.Pin1.PinName = Connected->GetName().ToString();
					Connection.Pin2.NodeID = SerializedNode.NodeID;
					Connection.Pin2.PinName = Pin->GetName().ToString();
					Graph.PinConnections.Add(MoveTemp(Connection));
				}
			}
			SerializedNode.Pins.Add(MoveTemp(SerializedPin));
		}
		int Index = Graph.Nodes.Add(MoveTemp(SerializedNode));
		SerializedNodes.Add(Node, Index);
	}

	for (FFIVSSerializedNode& Node : Graph.Nodes) {
		Node.NodePos -= LargestOffset;
	}

	FJsonObjectConverter::CustomExportCallback ExportCallback;
	ExportCallback.BindLambda([](FProperty* InProp, const void* InVal) -> TSharedPtr<FJsonValue> {
		if (InProp->IsA<FStructProperty>() && Cast<FStructProperty>(InProp)->Struct == FFIVSSerializedPin::StaticStruct()) {
			const FFIVSSerializedPin& SerializedPin = *static_cast<const FFIVSSerializedPin*>(InVal);
			
			if (SerializedPin.PinLiteralValue.IsValid()) {
				TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
				Obj->SetStringField(TEXT("pinName"), SerializedPin.PinName);
				Obj->SetField(TEXT("pinLiteral"), SerializedPin.PinLiteralValue);
				return MakeShared<FJsonValueObject>(Obj);
			}
			return nullptr;
		}
		return nullptr;
	});
	FString JsonString;
	FJsonObjectConverter::UStructToFormattedJsonObjectString<TCHAR, TPrettyJsonPrintPolicy>(FFIVSSerializedGraph::StaticStruct(), &Graph, JsonString, 0, 0, 0, &ExportCallback);

	return JsonString;
}

FString UFIVSSerailizationUtils::FIVS_SerailizeGraph(UFIVSGraph* Graph) {
	FString Str = FIVS_SerializePartial(Graph->GetNodes(), false);
	UE_LOG(LogFicsItNetworks, Warning, TEXT("%s"), *Str);
	return Str;
}

#pragma optimize("", off)
void UFIVSSerailizationUtils::FIVS_DeserializeGraph(UFIVSGraph* Graph, FString InStr, FVector2D InOffset) {
	FFIVSSerializedGraph SerializedGraph;
	TMap<int, UFIVSNode*> NodeIDs;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(InStr);
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid()) {
		const TArray<TSharedPtr<FJsonValue>>* PinConnections;
		if (JsonObject->TryGetArrayField(TEXT("pinConnections"), PinConnections)) {
			FJsonObjectConverter::JsonArrayToUStruct(*PinConnections, &SerializedGraph.PinConnections, 0, 0);
		}
		const TArray<TSharedPtr<FJsonValue>>* Nodes;
		if (JsonObject->TryGetArrayField(TEXT("nodes"), Nodes)) {
			for (const TSharedPtr<FJsonValue>& JsonNode : *Nodes) {
				const TSharedPtr<FJsonObject>* Node;
				if (JsonNode->TryGetObject(Node)) {
					FFIVSSerializedNode SerializedNode;
					(*Node)->TryGetNumberField(TEXT("nodeID"), SerializedNode.NodeID);
					const TSharedPtr<FJsonObject>* NodePos;
					if ((*Node)->TryGetObjectField(TEXT("nodePos"), NodePos)) {
						double x = 0, y = 0;
						(*NodePos)->TryGetNumberField(TEXT("x"), x);
						(*NodePos)->TryGetNumberField(TEXT("y"), y);
						SerializedNode.NodePos = FVector2D(x, y);
					}
					TSharedPtr<FJsonValue> NodeType = (*Node)->TryGetField(TEXT("nodeType"));
					if (!FJsonObjectConverter::JsonValueToUProperty(NodeType, FindFProperty<FClassProperty>(FFIVSSerializedNode::StaticStruct(), TEXT("NodeType")), &SerializedNode.NodeType, 0, 0)) {
						continue;
					}
					const TSharedPtr<FJsonObject>* Properties;
					if ((*Node)->TryGetObjectField(TEXT("properties"), Properties)) {
						FJsonObjectConverter::JsonObjectToUStruct(Properties->ToSharedRef(), FFIVSNodeProperties::StaticStruct(), &SerializedNode.Properties, 0, 0);
					}
					const TArray<TSharedPtr<FJsonValue>>* Pins;
					if ((*Node)->TryGetArrayField(TEXT("pins"), Pins)) {
						for (const TSharedPtr<FJsonValue>& Pin : *Pins) {
							const TSharedPtr<FJsonObject>* Obj;
							if (!Pin->TryGetObject(Obj)) continue;
							FFIVSSerializedPin SerializedPin;
							(*Obj)->TryGetStringField(TEXT("pinName"), SerializedPin.PinName); 
							SerializedPin.PinLiteralValue = (*Obj)->TryGetField(TEXT("pinLiteral"));
							SerializedNode.Pins.Add(MoveTemp(SerializedPin));
						}
					}
					SerializedGraph.Nodes.Add(MoveTemp(SerializedNode));
				}
			}
		}
	}

	for (const FFIVSSerializedNode& SerializedNode : SerializedGraph.Nodes) {
		UFIVSNode* Node = NewObject<UFIVSNode>(Graph, SerializedNode.NodeType);
		Node->Pos = SerializedNode.NodePos + InOffset;
		Node->DeserializeNodeProperties(SerializedNode.Properties);
		Node->InitPins();
		NodeIDs.Add(SerializedNode.NodeID, Node);
		TArray<UFIVSPin*> Pins = Node->GetNodePins();
		for (const FFIVSSerializedPin& SerializedPin : SerializedNode.Pins) {
			UFIVSPin** Pin = Pins.FindByPredicate([&SerializedPin](UFIVSPin* Pin) {
				return Pin->GetName().ToString() == SerializedPin.PinName;
			});
			if (Pin) {
				FIVS_SetPinLiteralFromJson(*Pin, SerializedPin.PinLiteralValue);
			}
		}
		Graph->AddNode(Node);
	}

	for (const FFIVSSerializedPinConnection& SerializedPinConnection : SerializedGraph.PinConnections) {
		UFIVSNode** Node1 = NodeIDs.Find(SerializedPinConnection.Pin1.NodeID);
		UFIVSNode** Node2 = NodeIDs.Find(SerializedPinConnection.Pin2.NodeID);
		if (!Node1 || !Node2) continue;
		TArray<UFIVSPin*> Pins1 = (*Node1)->GetNodePins();
		UFIVSPin** Pin1 = Pins1.FindByPredicate([&SerializedPinConnection](UFIVSPin* Pin) {
			return Pin->GetName().ToString() == SerializedPinConnection.Pin1.PinName;
		});
		TArray<UFIVSPin*> Pins2 = (*Node2)->GetNodePins();
		UFIVSPin** Pin2 = Pins2.FindByPredicate([&SerializedPinConnection](UFIVSPin* Pin) {
			return Pin->GetName().ToString() == SerializedPinConnection.Pin2.PinName;
		});
		if (!Pin1 || !Pin2 || !*Pin1 || !*Pin2) continue;
		(*Pin1)->AddConnection(*Pin2);
	}
}
#pragma optimize("", on)

