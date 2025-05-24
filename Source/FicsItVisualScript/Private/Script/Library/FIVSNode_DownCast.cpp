#include "Script/Library/FIVSNode_DownCast.h"

#include "FIVSEdEditor.h"
#include "JsonObject.h"

void UFIVSNode_DownCast::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	UFIRClass* FromClass = FFicsItReflectionModule::Get().FindClass(UObject::StaticClass());
	for (TPair<UClass*, UFIRClass*> Class : FFicsItReflectionModule::Get().GetClasses()) {
		FFIVSNodeAction Action;
		Action.NodeType = UFIVSNode_DownCast::StaticClass();
		Action.Title = FText::FromString(TEXT("Cast to ") + Class.Value->GetDisplayName().ToString());
		Action.Category = FText::FromString(TEXT("Casts"));
		Action.SearchableText = Action.Title;
		Action.Pins = {
			{FIVS_PIN_DATA_INPUT, FFIVSPinDataType(FIR_TRACE, FromClass)},
			{FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIR_TRACE, Class.Value)},
			{FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIR_BOOL)}
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

	UFIRClass* Class = Cast<UFIRClass>(FSoftObjectPath(Properties->GetStringField(TEXT("ToClass"))).TryLoad());
	if (Class) SetClass(Class);
}

void UFIVSNode_DownCast::CompileNodeToLua(FFIVSLuaCompilerContext& Context) {
	Context.AddEntrance(ExecInput);
	FString input = Context.GetRValueExpression(DataInput);
	Context.AddRValue(DataOutput, input);
	FString className = ToClass->GetInternalName();
	FString isAExpression = FString::Printf(TEXT("(%s and %s:isA(classes.%s))"), *input, *input, *className);
	if (bPure) {
		Context.AddRValue(SuccessOutput, isAExpression);
	} else {
		Context.AddPlain(FString::Printf(TEXT("if %s then\n"), *isAExpression));
		Context.EnterNewSection();
		Context.ContinueCurrentSection(SuccessOutput);
		Context.LeaveSection();
		Context.AddPlain(TEXT("else\n"));
		Context.EnterNewSection();
		Context.ContinueCurrentSection(FailureOutput);
		Context.LeaveSection();
;		Context.AddPlain(TEXT("end\n"));
	}
}

void UFIVSNode_DownCast::SetClass(UFIRClass* InToClass) {
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
		DataInput = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("From"), FText::FromString(TEXT("From")), FFIVSPinDataType(FIR_TRACE, FFicsItReflectionModule::Get().FindClass(UObject::StaticClass())));
		DataOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("To"), FText::FromString(TEXT("To")), FFIVSPinDataType(FIR_TRACE, ToClass));
		SuccessOutput = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Success"), FText::FromString(TEXT("Success")), FFIVSPinDataType(FIR_BOOL));
	}
}
