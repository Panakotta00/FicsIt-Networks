#include "Script/Library/FIVSNode_Variable.h"

#include "JsonObjectConverter.h"
#include "Kernel/FIVSRuntimeContext.h"
#include "Kernel/FIVSValue.h"

void FFIVSNodeStatement_DeclareVariable::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(InitialValue);
}

void FFIVSNodeStatement_DeclareVariable::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	const FFIRAnyValue* Value = Context.TryGetRValue(InitialValue);
	Context.SetValue(VarOut, FFIVSValue::MakeLValue(VarOut, *Value));
}

void FFIVSNodeStatement_Assign::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(ValIn);
	Context.Push_EvaluatePin(VarIn);
}

void FFIVSNodeStatement_Assign::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	FFIRAnyValue* variable = Context.TryGetLValue(VarIn);
	if (!variable) return;

	const FFIRAnyValue* value = Context.TryGetRValue(ValIn);
	if (!value) return;

	*variable = *value;

	Context.Push_ExecPin(ExecOut);
}

void UFIVSNode_Variable::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (EFIRValueType DataType = FIR_NIL; DataType <= FIR_ANY; DataType = EFIRValueType(DataType + 1)) {
		// Input FIN_ANY is excluded from conversion because it may fail or not and needs its own node
		if (DataType >= FIR_OBJ || DataType == FIR_NIL) continue;
		FString Name = FIRGetNetworkValueTypeName(DataType);
		
		FFIVSNodeAction Action;
		Action.NodeType = UFIVSNode_Variable::StaticClass();

		Action.Title = FText::FromString(TEXT("Declare Local Variable (") + Name + TEXT(")"));
		Action.Category = FText::FromString(TEXT("General|Local"));
		Action.SearchableText = Action.Title;
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType)));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(DataType).AsRef()));
		Action.OnExecute.BindLambda([DataType](UFIVSNode* Node) {
			Cast<UFIVSNode_Variable>(Node)->SetType(FFIVSPinDataType(DataType), false);
		});
		Actions.Add(Action);

		Action.Title = FText::FromString(TEXT("Assign value to Reference (") + Name + TEXT(")"));
		Action.SearchableText = Action.Title;
		Action.Pins.Empty();
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_EXEC_INPUT));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_EXEC_OUTPUT));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType).AsRef()));
		Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(DataType)));
		Action.OnExecute.BindLambda([DataType](UFIVSNode* Node) {
			Cast<UFIVSNode_Variable>(Node)->SetType(FFIVSPinDataType(DataType), true);
		});
		Actions.Add(Action);
	}
}

void UFIVSNode_Variable::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetBoolField(TEXT("Assignment"), bAssignment);
	TSharedRef<FJsonObject> typeObj = MakeShared<FJsonObject>();
	FJsonObjectConverter::UStructToJsonObject(FFIVSPinDataType::StaticStruct(), &Type, typeObj);
	Properties->SetObjectField(TEXT("Type"), typeObj);
}

void UFIVSNode_Variable::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	bAssignment = Properties->GetBoolField(TEXT("Assignment"));
	const TSharedPtr<FJsonObject>& typeObj = Properties->GetObjectField(TEXT("Type"));
	if (typeObj) {
		FJsonObjectConverter::JsonObjectToUStruct(typeObj.ToSharedRef(), FFIVSPinDataType::StaticStruct(), &Type);
		SetType(Type, bAssignment);
	}
}

TFIRInstancedStruct<FFIVSNodeStatement> UFIVSNode_Variable::CreateNodeStatement() {
	if (bAssignment) {
		return FFIVSNodeStatement_Assign{
			NodeId,
			ExecInput->PinId,
			ExecOutput->PinId,
			VarPin->PinId,
			DataInput->PinId,
		};
	} else {
		return FFIVSNodeStatement_DeclareVariable{
			NodeId,
			VarPin->PinId,
			DataInput->PinId,
		};
	}
}

void UFIVSNode_Variable::CompileNodeToLua(FFIVSLuaCompilerContext& Context) const {
	if (bAssignment) {
		Context.AddEntrance(ExecInput);
		TOptional<FString> lvalue = Context.GetLValueExpression(VarPin);
		if (lvalue) {
			FString rvalue = Context.GetRValueExpression(DataInput);
			Context.AddPlain(FString::Printf(TEXT("%s = %s\n"), **lvalue, *rvalue));
		}
		Context.ContinueCurrentSection(ExecOutput);
	} else {
		FString data = Context.GetRValueExpression(DataInput);
		Context.AddOutputPinAsVariable(VarPin, data);
	}
}

void UFIVSNode_Variable::SetType(const FFIVSPinDataType& InType, bool bIsAssignment) {
	Type = InType;
	bAssignment = bIsAssignment;

	if (bAssignment) {
		DisplayName = FText::FromString(TEXT("Assign Reference"));
	} else {
		DisplayName = FText::FromString(TEXT("Declare Local Variable"));
	}

	DeletePin(ExecInput);
	DeletePin(ExecOutput);
	DeletePin(VarPin);
	DeletePin(DataInput);

	if (bAssignment) {
		ExecInput = CreatePin(FIVS_PIN_EXEC_INPUT,TEXT("Exec"), FText::FromString(TEXT("Exec")));
		ExecOutput = CreatePin(FIVS_PIN_EXEC_OUTPUT, TEXT("Out"), FText::FromString(TEXT("Out")));
		VarPin = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Var"), FText::FromString(TEXT("Var")), Type.AsRef());
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Value"), FText::FromString(TEXT("Value")), Type.AsVal());
	} else {
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Init Value"), FText::FromString(TEXT("Init Value")), Type.AsVal());
		VarPin = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Var"), FText::FromString(TEXT("Var")), Type.AsRef());
	}
}
