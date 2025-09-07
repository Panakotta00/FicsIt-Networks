#include "Script/FIVSNodeSerialization.h"

#include "FicsItVisualScriptModule.h"
#include "FIVSUtils.h"
#include "Script/FIVSGraph.h"
#include "Script/FIVSNode.h"
#include "JsonObjectConverter.h"
#include "Logging/StructuredLog.h"

TSharedPtr<FJsonValue> FIVS_GetPinLiteralAsJson(UFIVSPin* InPin) {
	if (InPin->GetPinType() & FIVS_PIN_EXEC) return nullptr;
	switch (InPin->GetPinDataType().GetType()) {
	case FIR_NIL:
		return nullptr;
	case FIR_BOOL:
		return MakeShared<FJsonValueBoolean>(InPin->GetLiteral().GetBool());
	case FIR_INT:
		return MakeShared<FJsonValueNumber>(InPin->GetLiteral().GetInt());
	case FIR_FLOAT:
		return MakeShared<FJsonValueNumber>(InPin->GetLiteral().GetFloat());
	case FIR_STR:
		return MakeShared<FJsonValueString>(InPin->GetLiteral().GetString());
	case FIR_OBJ:
		return MakeShared<FJsonValueString>(InPin->GetLiteral().GetObj().Get()->GetPathName());
	case FIR_CLASS:
		return MakeShared<FJsonValueString>(InPin->GetLiteral().GetClass()->GetPathName());
	case FIR_TRACE:
		return MakeShared<FJsonValueString>(UFIVSUtils::NetworkTraceToString(InPin->GetLiteral().GetTrace()));
	case FIR_STRUCT:
		return nullptr; // TODO: Maybe add Struct serialization here, but maybe this is broken up before hand and pins can't have struct literals, instead the pins may have to be split up into individual pins which can then have a literal
	case FIR_ARRAY:
		return nullptr; // TODO: Maybe add Array serialization, but pins would actually have to support it
	case FIR_ANY:
		return nullptr; // TODO: Pins with FIRAny... I have no clue, is that even possible?
	default: ;
	}
	return nullptr;
}

void FIVS_SetPinLiteralFromJson(UFIVSPin* InPin, TSharedPtr<FJsonValue> InValue) {
	if (InPin->GetPinType() & FIVS_PIN_EXEC) return;
	switch (InPin->GetPinDataType().GetType()) {
	case FIR_NIL:
		return;
	case FIR_BOOL:
		return InPin->SetLiteral(InValue->AsBool());
	case FIR_INT:
		return InPin->SetLiteral((FIRInt)InValue->AsNumber());
	case FIR_FLOAT:
		return InPin->SetLiteral(InValue->AsNumber());
	case FIR_STR:
		return InPin->SetLiteral(InValue->AsString());
	case FIR_OBJ:
		return InPin->SetLiteral((FIRObj)FSoftObjectPath(InValue->AsString()).TryLoad());
	case FIR_CLASS:
		return InPin->SetLiteral(Cast<UClass>(FSoftObjectPath(InValue->AsString()).TryLoad()));
	case FIR_TRACE:
		return InPin->SetLiteral(UFIVSUtils::StringToNetworkTrace(InValue->AsString()));
	case FIR_STRUCT:
		// TODO: Maybe add Struct serialization here, but maybe this is broken up before hand and pins can't have struct literals, instead the pins may have to be split up into individual pins which can then have a literal
	case FIR_ARRAY:
		// TODO: Maybe add Array serialization, but pins would actually have to support it
	case FIR_ANY:
		// TODO: Pins with FIRAny... I have no clue, is that even possible?
	default: ;
	}
}

FString UFIVSSerailizationUtils::FIVS_SerializePartial(TArray<UFIVSNode*> InNodes, bool bZeroOffset) {
	TSharedRef<FJsonObject> graph = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> nodes;
	TArray<TSharedPtr<FJsonValue>> pinConnections;
	TSet<TPair<UFIVSPin*,UFIVSPin*>> serializedPinConnections;

	for (UFIVSNode* node : InNodes) {
		TSharedRef<FJsonObject> nodeObject = MakeShared<FJsonObject>();

		nodeObject->SetStringField(TEXT("type"), node->GetClass()->GetPathName());

		nodeObject->SetStringField(TEXT("id"), node->NodeId.ToString());

		TSharedRef<FJsonObject> pos = MakeShared<FJsonObject>();
		FJsonObjectConverter::UStructToJsonObject(TBaseStructure<FVector2D>::Get(), &node->Pos, pos);
		nodeObject->SetObjectField(TEXT("pos"), pos);

		TSharedRef<FJsonObject> properties = MakeShared<FJsonObject>();
		node->SerializeNodeProperties(properties);
		nodeObject->SetObjectField(TEXT("props"), properties);

		TArray<TSharedPtr<FJsonValue>> pins;
		for (UFIVSPin* Pin : node->GetNodePins()) {
			TSharedRef<FJsonObject> pinObject = MakeShared<FJsonObject>();

			pinObject->SetStringField(TEXT("id"), Pin->PinId.ToString());
			pinObject->SetStringField(TEXT("name"), Pin->GetName());
			TSharedPtr<FJsonValue> literal = FIVS_GetPinLiteralAsJson(Pin);
			if (literal) {
				pinObject->SetField(TEXT("literal"), literal);
			}

			pins.Add(MakeShared<FJsonValueObject>(pinObject));

			for (UFIVSPin* Connected : Pin->GetConnections()) {
				if (serializedPinConnections.Contains(TPair<UFIVSPin*,UFIVSPin*>(Connected, Pin))) continue;

				serializedPinConnections.Add({Connected, Pin});
				serializedPinConnections.Add({Pin, Connected});

				TSharedRef<FJsonObject> pinConnection = MakeShared<FJsonObject>();
				pinConnection->SetStringField(TEXT("from"), Pin->PinId.ToString());
				pinConnection->SetStringField(TEXT("to"), Connected->PinId.ToString());

				pinConnections.Add(MakeShared<FJsonValueObject>(pinConnection));
			}
		}
		nodeObject->SetArrayField(TEXT("pins"), pins);

		nodes.Add(MakeShared<FJsonValueObject>(nodeObject));
	}

	graph->SetArrayField(TEXT("nodes"), nodes);
	graph->SetArrayField(TEXT("connections"), pinConnections);

	FString JsonString;
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> jsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonString);

	if (FJsonSerializer::Serialize(graph, jsonWriter)) {
		jsonWriter->Close();
		return JsonString;
	} else {
		UE_LOG(LogFicsItVisualScript, Warning, TEXT("Unable to serialize graph!"));
		jsonWriter->Close();
	}

	return TEXT("");
}

FString UFIVSSerailizationUtils::FIVS_SerailizeGraph(UFIVSGraph* Graph) {
	FString Str = FIVS_SerializePartial(Graph->GetNodes(), false);
	return Str;
}

TArray<UFIVSNode*> UFIVSSerailizationUtils::FIVS_DeserializeGraph(UFIVSGraph* Graph, FString InStr, bool bCreateNewGuids) {
	TSharedRef<FJsonStringReader> jsonReader = FJsonStringReader::Create(InStr);
	TSharedPtr<FJsonObject> graphObject;
	if (!FJsonSerializer::Deserialize<TCHAR>(jsonReader, graphObject)) {
		UE_LOG(LogFicsItVisualScript, Warning, TEXT("Unable to deserialize graph: %s"), *jsonReader->GetErrorMessage());
		return {};
	}

	TArray<UFIVSNode*> deserializedNodes;
	TMap<FGuid, UFIVSPin*> deserializedPins;

	const TArray<TSharedPtr<FJsonValue>>* nodes = nullptr;
	if (graphObject->TryGetArrayField(TEXT("nodes"), nodes)) {
		for (const TSharedPtr<FJsonValue>& nodeValue : *nodes) {
			const TSharedPtr<FJsonObject>* nodeObjectPtr = nullptr;
			if (!nodeValue->TryGetObject(nodeObjectPtr)) {
				continue;
			}
			const FJsonObject* nodeObject = nodeObjectPtr->Get();

			FString typeStr;
			if (!nodeObject->TryGetStringField(TEXT("type"), typeStr)) {
				// TODO: Create fake Node to prevent data-loss
				continue;
			}

			TSubclassOf<UFIVSNode> type = Cast<UClass>(FSoftObjectPath(typeStr).TryLoad());
			if (!type) {
				// TODO: Create fake Node to prevent data-loss
				continue;
			}

			UFIVSNode* node = NewObject<UFIVSNode>(Graph, type);

			FString idStr;
			if (!bCreateNewGuids && nodeObject->TryGetStringField(TEXT("id"), idStr)) {
				FGuid::Parse(idStr, node->NodeId);
			}
			deserializedNodes.Add(node);

			const TSharedPtr<FJsonObject>* posObj;
			if (nodeObject->TryGetObjectField(TEXT("pos"), posObj)) {
				// TODO: Use JsonObjectToUStruct
				//FJsonObjectConverter::JsonObjectToUStruct(posObj->ToSharedRef(), TBaseStructure<FVector2D>::Get(), &node->Pos);
				(*posObj)->TryGetNumberField(TEXT("X"), node->Pos.X);
				(*posObj)->TryGetNumberField(TEXT("Y"), node->Pos.Y);
			}

			const TSharedPtr<FJsonObject>& props = nodeObject->GetObjectField(TEXT("props"));
			node->DeserializeNodeProperties(props);

			const TArray<TSharedPtr<FJsonValue>>* pins;
			if (nodeObject->TryGetArrayField(TEXT("pins"), pins)) {
				for (const TSharedPtr<FJsonValue>& pinValue : *pins) {
					const TSharedPtr<FJsonObject>* pinObjPtr = nullptr;
					if (!pinValue->TryGetObject(pinObjPtr)) {
						continue;
					}
					const FJsonObject* pinObj = pinObjPtr->Get();

					FString name;
					if (!pinObj->TryGetStringField(TEXT("name"), name)) {
						continue;
					}

					UFIVSPin* pin = node->FindPinByName(name);
					if (!pin) {
						// TODO: Add pin that was not found anymore (prevent data loss)
						continue;
					}

					FGuid id;
					if (pinObj->TryGetStringField(TEXT("id"), idStr) && FGuid::Parse(idStr, id)) {
						deserializedPins.Add(id, pin);
						if (!bCreateNewGuids) {
							pin->PinId = id;
						}
					}

					TSharedPtr<FJsonValue> literal = pinObj->TryGetField(TEXT("literal"));
					if (literal) {
						FIVS_SetPinLiteralFromJson(pin, literal);
					}
				}
			}

			Graph->AddNode(node);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* pinConnections = nullptr;
	if (graphObject->TryGetArrayField(TEXT("connections"), pinConnections)) {
		for (const TSharedPtr<FJsonValue>& connection : *pinConnections) {
			const TSharedPtr<FJsonObject>* connObjPtr = nullptr;
			if (!connection->TryGetObject(connObjPtr)) {
				continue;
			}
			const FJsonObject* pinObj = connObjPtr->Get();

			FString fromStr;
			FGuid fromId;
			if (!pinObj->TryGetStringField(TEXT("from"), fromStr) || !FGuid::Parse(fromStr, fromId)) {
				continue;
			}

			FString toStr;
			FGuid toId;
			if (!pinObj->TryGetStringField(TEXT("to"), toStr) || !FGuid::Parse(toStr, toId)) {
				continue;
			}

			UFIVSPin** from = deserializedPins.Find(fromId);
			UFIVSPin** to = deserializedPins.Find(toId);
			if (!from || !to || !*from || !*to) {
				continue;
			}

			(*from)->AddConnection(*to);
		}
	}

	return deserializedNodes;
}

void UFIVSSerailizationUtils::FIVS_AdjustNodesOffset(const TArray<UFIVSNode*>& InNodes, FVector2D Offset, bool bRelativeToCenter) {
	FVector2D MaxPos = FVector2D(TNumericLimits<double>::Min());
	FVector2D MinPos = FVector2D(TNumericLimits<double>::Max());

	for (UFIVSNode* node : InNodes) {
		MaxPos = FVector2D::Max(MaxPos, node->Pos);
		MinPos = FVector2D::Min(MinPos, node->Pos);
	}

	FVector2D delta = -MinPos + Offset;
	if (bRelativeToCenter) {
		delta -= (MaxPos-MinPos)/2;
	}

	for (UFIVSNode* node : InNodes) {
		node->Pos += delta;
	}
}

