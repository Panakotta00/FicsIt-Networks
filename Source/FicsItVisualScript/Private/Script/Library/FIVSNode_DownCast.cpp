#include "Script/Library/FIVSNode_DownCast.h"

#include "Reflection/FINReflection.h"

void UFIVSNode_DownCast::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	UFINClass* FromClass = FFINReflection::Get()->FindClass(UObject::StaticClass());
	for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
		FFIVSNodeAction Action;
		Action.NodeType = UFIVSNode_DownCast::StaticClass();
		Action.Title = FText::FromString(TEXT("Cast to ") + Class.Value->GetDisplayName().ToString());
		Action.Category = FText::FromString(TEXT("Casts"));
		Action.SearchableText = Action.Title;
		Action.Pins = {
			{FIVS_PIN_DATA_INPUT, FFIVSPinDataType(FIN_TRACE, FromClass)},
			{FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIN_TRACE, Class.Value)},
			{FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIN_BOOL)}
		};
		Action.OnExecute.BindLambda([FromClass, Class](UFIVSNode* Node) {
			Cast<UFIVSNode_DownCast>(Node)->SetClass(Class.Value);
		});
		Actions.Add(Action);
	}
}

void UFIVSNode_DownCast::SerializeNodeProperties(FFIVSNodeProperties& Properties) const {
	Properties.Properties.Add(TEXT("ToClass"), ToClass->GetPathName());
}

void UFIVSNode_DownCast::DeserializeNodeProperties(const FFIVSNodeProperties& Properties) {
	ToClass = Cast<UFINClass>(FSoftObjectPath(Properties.Properties[TEXT("ToClass")]).TryLoad());
}

TArray<UFIVSPin*> UFIVSNode_DownCast::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return {DataInput};
}

UFIVSPin* UFIVSNode_DownCast::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFIVSValue Value = Context.GetValue(DataInput);
	FFINNetworkTrace Trace = Value->GetTrace();
	UObject* Obj = *Trace;
	bool bSuccess = Obj && Obj->IsA(Cast<UClass>(ToClass->GetOuter()));
	if (bPure) {
		Context.SetValue(SuccessOutput, bSuccess);
		Context.SetValue(DataOutput, Value);
	}
	return nullptr;
}

void UFIVSNode_DownCast::SetClass(UFINClass* InToClass) {
	ToClass = InToClass;

	DisplayName = FText::FromString(TEXT("Cast to ") + ToClass->GetDisplayName().ToString());

	if (bPure) {
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("From"), FText::FromString(TEXT("From")), FFIVSPinDataType(FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass())));
		DataOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("To"), FText::FromString(TEXT("To")), FFIVSPinDataType(FIN_TRACE, ToClass));
		SuccessOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Success"), FText::FromString(TEXT("Success")), FFIVSPinDataType(FIN_BOOL));
	}
}
