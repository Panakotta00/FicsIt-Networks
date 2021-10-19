#include "FIVSNode_DownCast.h"

#include "FicsItNetworks/Reflection/FINReflection.h"

TArray<FFIVSNodeAction> UFIVSNode_DownCast::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;

	UFINClass* FromClass = FFINReflection::Get()->FindClass(UObject::StaticClass());
	for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
		FFIVSNodeAction Action;
		Action.NodeType = UFIVSNode_DownCast::StaticClass();
		Action.Title = FText::FromString(TEXT("Cast to ") + Class.Value->GetDisplayName().ToString());
		Action.Category = FText::FromString(TEXT("Casts"));
		Action.SearchableText = Action.Title;
		Action.Pins = {
			{FIVS_PIN_DATA_INPUT, {FIN_TRACE, FromClass}},
			{FIVS_PIN_DATA_OUTPUT, {FIN_TRACE, Class.Value}},
			{FIVS_PIN_DATA_OUTPUT, FIN_BOOL}
		};
		Action.OnExecute.BindLambda([FromClass, Class](UFIVSNode* Node) {
			Cast<UFIVSNode_DownCast>(Node)->ToClass = Class.Value;
		});
		Actions.Add(Action);
	}
	return Actions;
}

void UFIVSNode_DownCast::InitPins() {
	if (bPure) {
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("From")), {FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass())});
		DataOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString(TEXT("To")), {FIN_TRACE, ToClass});
		SuccessOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString(TEXT("Success")), {FIN_BOOL});
	}
}

FString UFIVSNode_DownCast::GetNodeName() const {
	return TEXT("Cast to ") + ToClass->GetDisplayName().ToString();
}

TArray<UFIVSPin*> UFIVSNode_DownCast::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return {DataInput};
}

UFIVSPin* UFIVSNode_DownCast::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFINNetworkTrace Trace = Context.GetValue(DataInput).GetTrace();
	UObject* Obj = *Trace;
	bool bSuccess = Obj && Obj->IsA(Cast<UClass>(ToClass->GetOuter()));
	if (bPure) {
		Context.SetValue(SuccessOutput, bSuccess);
		Context.SetValue(DataOutput, Trace);
	}
	return nullptr;
}
