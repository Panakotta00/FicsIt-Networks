#include "Script/Library/FIVSNode_Literal.h"

#include "FastReferenceCollector.h"
#include "JsonObject.h"

void UFIVSNode_Literal::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (EFIRValueType DataType = FIR_NIL; DataType <= FIR_ANY; DataType = EFIRValueType(DataType + 1)) {
		// Input FIN_ANY is excluded from conversion because it may fail or not and needs its own node
		if (DataType > FIR_TRACE || DataType == FIR_CLASS || DataType == FIR_NIL) continue;
		FString Name = FIRGetNetworkValueTypeName(DataType);
		
		FFIVSNodeAction Action;
		Action.NodeType = UFIVSNode_Literal::StaticClass();

		Action.Title = FText::FromString(TEXT("Create Literal (") + Name + TEXT(")"));
		Action.Category = FText::FromString(TEXT("General|Literal"));
		Action.SearchableText = Action.Title;
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType, nullptr)));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(DataType, nullptr).AsRef()));
		Action.OnExecute.BindLambda([DataType](UFIVSNode* Node) {
			Cast<UFIVSNode_Literal>(Node)->SetType(DataType);
		});
		Actions.Add(Action);
	}
}

void UFIVSNode_Literal::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetStringField(TEXT("Type"), StaticEnum<EFIRValueType>()->GetValueAsString(Type));
}

void UFIVSNode_Literal::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	SetType((EFIRValueType)StaticEnum<EFIRValueType>()->GetValueByNameString(Properties->GetStringField(TEXT("Type"))));
}

void UFIVSNode_Literal::CompileNodeToLua(FFIVSLuaCompilerContext& Context) {
	FString data = Context.GetRValueExpression(Input);
	Context.AddRValue(Output, data);
}

void UFIVSNode_Literal::SetType(TEnumAsByte<EFIRValueType> InType) {
	Type = InType;

	DisplayName = FText::FromString(TEXT("Literal (") + FIRGetNetworkValueTypeName(Type) + TEXT(")"));

	DeletePin(Input);
	DeletePin(Output);

	Output = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Var"), FText::FromString(TEXT("Var")), FFIRExtendedValueType(Type, nullptr));
	Input = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Init Value"), FText::FromString(TEXT("Init Value")), FFIRExtendedValueType(Type, nullptr));
	Input->OnLiteralChanged.BindWeakLambda(this, [this]() {
		FFIRAnyValue Literal = Input->GetLiteral();
		UFIRStruct* SubType = nullptr;
		switch (Literal.GetType()) {
			case FIR_OBJ:
			case FIR_TRACE: {
				UObject* obj = Literal.GetObj().Get();
				SubType = obj ? FFicsItReflectionModule::Get().FindClass(obj->GetClass()) : nullptr;

				if (Output->GetPinDataType().GetRefSubType() != SubType) {
					RecreatePin(Output, FIVS_PIN_DATA_OUTPUT, TEXT("Var"), FText::FromString(TEXT("Var")), FFIRExtendedValueType(Type, SubType));
				}
			}
			default: break;
		}

	});
}
