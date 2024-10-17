#include "Script/Library/FIVSNode_SeparateStruct.h"

#include "FicsItReflection.h"
#include "FIRStruct.h"
#include "Kernel/FIVSRuntimeContext.h"

void FFIVSNodeStatement_SeparateStruct::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	TArray<FGuid> inputs;
	InputPins.GenerateValueArray(inputs);
	Context.Push_EvaluatePin(inputs);
}

void FFIVSNodeStatement_SeparateStruct::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	if (bBreak) {
		const FFIRAnyValue* value = Context.TryGetRValue(InputPins[TEXT("Struct")]);
		if (!value) return;
		FFIRInstancedStruct StructObj = value->GetStruct();
		FFIRExecutionContext Ctx(StructObj.GetData());
		for (UFIRProperty* Prop : Struct->GetProperties()) {
			if (!(Prop->GetPropertyFlags() & FIR_Prop_Attrib)) continue;
			const FGuid* Pin = OutputPins.Find(Prop->GetInternalName());
			if (Pin) Context.SetValue(*Pin, Prop->GetValue(Ctx));
		}
	} else {
		FFIRInstancedStruct StructObj(Cast<UScriptStruct>(Struct->GetOuter()));
		FFIRExecutionContext Ctx(StructObj.GetData());
		for (UFIRProperty* Prop : Struct->GetProperties()) {
			if (!(Prop->GetPropertyFlags() & FIR_Prop_Attrib)) continue;
			const FGuid* Pin = InputPins.Find(Prop->GetInternalName());
			if (Pin) Prop->SetValue(Ctx, *Context.TryGetRValue(*Pin));
		}
		Context.SetValue(OutputPins[TEXT("Struct")], StructObj);
	}
}

void UFIVSNode_SeparateStruct::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (TTuple<UScriptStruct*, UFIRStruct*> StructPair : FFicsItReflectionModule::Get().GetStructs()) {
		FFIVSNodeAction BreakAction;
		BreakAction.NodeType = UFIVSNode_SeparateStruct::StaticClass();
		BreakAction.Category = FText::FromString(TEXT("Struct"));
		BreakAction.Title = FText::FromString(TEXT("Break ") + StructPair.Value->GetDisplayName().ToString());
		for (UFIRProperty* Prop : StructPair.Value->GetProperties()) {
			FFIVSFullPinType FullPinType(Prop);
			FullPinType.PinType = FIVS_PIN_DATA_OUTPUT;
			BreakAction.Pins.Add(FullPinType);
		}
		BreakAction.SearchableText = BreakAction.Title;
		BreakAction.OnExecute.BindLambda([StructPair](UFIVSNode* Node) {
			Cast<UFIVSNode_SeparateStruct>(Node)->bBreak = true;
			Cast<UFIVSNode_SeparateStruct>(Node)->SetStruct(StructPair.Value);
		});
		FFIVSNodeAction MakeAction(BreakAction);
		
		BreakAction.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(FIR_STRUCT, StructPair.Value)));
		Actions.Add(BreakAction);

		for (FFIVSFullPinType& Pin : MakeAction.Pins) Pin.PinType = FIVS_PIN_DATA_INPUT;
		MakeAction.Title = FText::FromString(TEXT("Make ") + StructPair.Value->GetDisplayName().ToString());
		MakeAction.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIR_STRUCT, StructPair.Value)));
		MakeAction.SearchableText = MakeAction.Title;
		MakeAction.OnExecute.BindLambda([StructPair](UFIVSNode* Node) {
			Cast<UFIVSNode_SeparateStruct>(Node)->bBreak = false;
			Cast<UFIVSNode_SeparateStruct>(Node)->SetStruct(StructPair.Value);
		});
		Actions.Add(MakeAction);
	}
}

void UFIVSNode_SeparateStruct::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetStringField("Struct", Struct->GetPathName());
	Properties->SetBoolField("Break", bBreak);
}

void UFIVSNode_SeparateStruct::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	bBreak = Properties->GetBoolField("Break");
	SetStruct(Cast<UFIRStruct>(FSoftObjectPath(Properties->GetStringField(TEXT("Struct"))).TryLoad()));
}

void UFIVSNode_SeparateStruct::SetStruct(UFIRStruct* InStruct) {
	Struct = InStruct;

	if (bBreak) DisplayName = FText::FromString(TEXT("Break ") + Struct->GetDisplayName().ToString());
	else DisplayName = FText::FromString(TEXT("Make ") + Struct->GetDisplayName().ToString());

	for (auto [_, pin] : InputPins) DeletePin(pin);
	for (auto [_, pin] : OutputPins) DeletePin(pin);

	EFIVSPinType PinType = bBreak ? FIVS_PIN_DATA_OUTPUT : FIVS_PIN_DATA_INPUT;
	for (UFIRProperty* Prop : Struct->GetProperties()) {
		UFIVSPin* Pin = CreatePin(PinType, Prop->GetInternalName(), Prop->GetDisplayName(), FFIVSPinDataType(Prop));
		(bBreak ? OutputPins : InputPins).Add(Prop->GetInternalName(), Pin);
	}
	UFIVSPin* Pin = CreatePin(bBreak ? FIVS_PIN_DATA_INPUT : FIVS_PIN_DATA_OUTPUT, TEXT("Struct"), FText::FromString(TEXT("Struct")), FFIVSPinDataType(FIR_STRUCT, Struct));
	(bBreak ? InputPins : OutputPins).Add(TEXT("Struct"), Pin);
}
