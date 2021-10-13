#include "FIVSNode_DownCast.h"

TArray<FFIVSNodeAction> UFIVSNode_DownCast::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;

	TFunction<void(UFINClass*)> AddChildren;
	AddChildren = [&Actions, &AddChildren](UFINClass* Class) {
		for (UFINClass* ToClass : Class->GetChildClasses()) {
			UFINClass* FromClass = ToClass->GetParentClass();
			while (FromClass) {
				FFIVSNodeAction Action;
				Action.NodeType = UFIVSNode_DownCast::StaticClass();
				Action.Title = FText::FromString(TEXT("Cast ") + FromClass->GetDisplayName().ToString() + TEXT(" to ") + ToClass->GetDisplayName().ToString());
				Action.Category = FText::FromString(TEXT("Casts"));
				Action.SearchableText = Action.Title;
				Action.Pins = {
					{FIVS_PIN_DATA_INPUT, {FIN_TRACE, FromClass}},
					{FIVS_PIN_DATA_OUTPUT, {FIN_TRACE, ToClass}},
					{FIVS_PIN_DATA_OUTPUT, FIN_BOOL}
				};
				Action.OnExecute.BindLambda([FromClass, ToClass](UFIVSNode* Node) {
					Cast<UFIVSNode_DownCast>(Node)->FromClass = FromClass;
					Cast<UFIVSNode_DownCast>(Node)->ToClass = ToClass;
				});
				Actions.Add(Action);
				
				FromClass = FromClass->GetParentClass();
			}
			AddChildren(ToClass);
		}
	};
	AddChildren(FFINReflection::Get()->FindClass(UObject::StaticClass()));
	return Actions;
}

void UFIVSNode_DownCast::InitPins() {
	if (bPure) {
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("From")), {FIN_TRACE, FromClass});
		DataOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString(TEXT("To")), {FIN_TRACE, ToClass});
		SuccessOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString(TEXT("Success")), {FIN_BOOL});
	}
}

FString UFIVSNode_DownCast::GetNodeName() const {
	return TEXT("Cast ") + FromClass->GetDisplayName().ToString() + TEXT(" to ") + ToClass->GetDisplayName().ToString();
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
