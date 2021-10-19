#include "FIVSNodeSerialization.h"

#include "FicsItNetworks/FicsItNetworksModule.h"
#include "FIVSGraph.h"
#include "FIVSNode.h"
#include "JsonObjectConverter.h"

TSharedPtr<FJsonValue> FIVS_GetPinLiteralAsJson(UFIVSPin* InPin) {
	if (InPin->GetPinType() & FIVS_PIN_DATA && !(InPin->GetPinType() & FIVS_PIN_EXEC)) return nullptr;
	switch (InPin->GetPinDataType().GetType()) {
	case FIN_NIL:
		return MakeShared<FJsonValueNull>();
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
		return MakeShared<FJsonValueNull>(); // TODO: Trace to JSON
	case FIN_STRUCT:
		return MakeShared<FJsonValueNull>(); // TODO: Maybe add Struct serialization here, but maybe this is broken up before hand and pins can't have struct literals, instead the pins may have to be split up into individual pins which can then have a literal
	case FIN_ARRAY:
		return MakeShared<FJsonValueNull>(); // TODO: Maybe add Array serialization, but pins would actually have to support it
	case FIN_ANY:
		return MakeShared<FJsonValueNull>(); // TODO: Pins with FINAny... I have no clue, is that even possible?
	default: ;
	}
	return MakeShared<FJsonValueNull>();
}

void FIVS_SetPinLiteralFromJson(UFIVSPin* InPin, TSharedPtr<FJsonValue> InValue) {
	if (InPin->GetPinType() & FIVS_PIN_DATA && !(InPin->GetPinType() & FIVS_PIN_EXEC)) return;
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

FString FIVS_SerializePartial(TArray<UFIVSNode*> InNodes) {
	FFIVSSerializedGraph Graph;
	TMap<UFIVSNode*, int> SerializedNodes;
	for (UFIVSNode* Node : InNodes) {
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

	FJsonObjectConverter::CustomExportCallback ExportCallback;
	ExportCallback.BindLambda([](FProperty* InProp, const void* InVal) -> TSharedPtr<FJsonValue> {
		if (InProp->IsA<FStructProperty>() && Cast<FStructProperty>(InProp)->Struct == FFIVSSerializedPin::StaticStruct()) {
			const FFIVSSerializedPin& SerializedPin = *static_cast<const FFIVSSerializedPin*>(InVal);
			TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
			Obj->SetStringField(TEXT("pinName"), SerializedPin.PinName);
			Obj->SetField(TEXT("pinLiteral"), SerializedPin.PinLiteralValue);
			return MakeShared<FJsonValueObject>(Obj);
		}
		return nullptr;
	});
	FString JsonString;
	FJsonObjectConverter::UStructToFormattedJsonObjectString<TCHAR, TPrettyJsonPrintPolicy>(FFIVSSerializedGraph::StaticStruct(), &Graph, JsonString, 0, 0, 0, &ExportCallback);

	return JsonString;
}

FString UFIVSSerailizationUtils::FIVS_SerailizeGraph(UFIVSGraph* Graph) {
	FString Str = FIVS_SerializePartial(Graph->GetNodes());
	UE_LOG(LogFicsItNetworks, Warning, TEXT("%s"), *Str);
	return Str;
}

void UFIVSSerailizationUtils::FIVS_DeserializeGraph(UFIVSGraph* Graph, FString InStr, FVector2D InOffset) {
	FFIVSSerializedGraph SerializedGraph;
	TMap<int, UFIVSNode*> Nodes;

	FJsonObjectConverter::JsonObjectStringToUStruct(InStr, &SerializedGraph, 0, 0);
	for (const FFIVSSerializedNode& SerializedNode : SerializedGraph.Nodes) {
		UFIVSNode* Node = NewObject<UFIVSNode>(Graph, SerializedNode.NodeType);
		Node->Pos = SerializedNode.NodePos + InOffset;
		Node->DeserializeNodeProperties(SerializedNode.Properties);
		Node->InitPins();
		Nodes.Add(SerializedNode.NodeID, Node);
		TArray<UFIVSPin*> Pins = Node->GetNodePins();
		for (const FFIVSSerializedPin& SerializedPin : SerializedNode.Pins) {
			UFIVSPin** Pin = Pins.FindByPredicate([&SerializedPin](UFIVSPin* Pin) {
				return Pin->GetName().ToString() == SerializedPin.PinName;
			});
			if (Pin) {
				FIVS_SetPinLiteralFromJson(*Pin, SerializedPin.PinLiteralValue);
			}
		}
	}

	for (const FFIVSSerializedPinConnection& SerializedPinConnection : SerializedGraph.PinConnections) {
		UFIVSNode** Node1 = Nodes.Find(SerializedPinConnection.Pin1.NodeID);
		UFIVSNode** Node2 = Nodes.Find(SerializedPinConnection.Pin2.NodeID);
		if (!Node1 || !Node2) continue;
		UFIVSPin** Pin1 = (*Node1)->GetNodePins().FindByPredicate([&SerializedPinConnection](UFIVSPin* Pin) {
			return Pin->GetName().ToString() == SerializedPinConnection.Pin1.PinName;
		});
		UFIVSPin** Pin2 = (*Node1)->GetNodePins().FindByPredicate([&SerializedPinConnection](UFIVSPin* Pin) {
			return Pin->GetName().ToString() == SerializedPinConnection.Pin2.PinName;
		});
		if (!Pin1 || !Pin2) continue;
		(*Pin1)->AddConnection(*Pin2);
	}
}