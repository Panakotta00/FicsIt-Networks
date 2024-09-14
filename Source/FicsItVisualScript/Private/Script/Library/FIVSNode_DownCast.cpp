#include "Script/Library/FIVSNode_DownCast.h"

#include "Kernel/FIVSRuntimeContext.h"
#include "Reflection/FINReflection.h"

void FFIVSNodeStatement_Cast::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(DataIn);
}

void FFIVSNodeStatement_Cast::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	const FFINAnyNetworkValue* value = Context.TryGetRValue(DataIn);
	FFINNetworkTrace Trace = value->GetTrace();
	UObject* Obj = *Trace;
	bool bSuccess = Obj && Obj->IsA(Cast<UClass>(ToClass->GetOuter()));
	if (bSuccess) {
		Context.SetValue(DataOut, *value);
	}
	if (FailureOut.IsValid()) {
		if (bSuccess) {
			Context.Push_ExecPin(SuccessOut);
		} else {
			Context.Push_ExecPin(FailureOut);
		}
	} else {
		Context.SetValue(SuccessOut, bSuccess);
	}
}

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

void UFIVSNode_DownCast::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetStringField(TEXT("ToClass"), ToClass->GetPathName());
}

void UFIVSNode_DownCast::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	if (!Properties) return;

	SetClass(Cast<UFINClass>(FSoftObjectPath(Properties->GetStringField(TEXT("ToClass"))).TryLoad()));
}

void UFIVSNode_DownCast::SetClass(UFINClass* InToClass) {
	ToClass = InToClass;

	if (ToClass->GetDisplayName().IsEmpty()) {
		DisplayName = FText::FromString(TEXT("Cast to ") + ToClass->GetInternalName());
	} else {
		DisplayName = FText::FromString(TEXT("Cast to ") + ToClass->GetDisplayName().ToString());
	}

	DeletePin(DataInput);
	DeletePin(DataOutput);
	DeletePin(SuccessOutput);

	if (bPure) {
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("From"), FText::FromString(TEXT("From")), FFIVSPinDataType(FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass())));
		DataOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("To"), FText::FromString(TEXT("To")), FFIVSPinDataType(FIN_TRACE, ToClass));
		SuccessOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Success"), FText::FromString(TEXT("Success")), FFIVSPinDataType(FIN_BOOL));
	}
}
