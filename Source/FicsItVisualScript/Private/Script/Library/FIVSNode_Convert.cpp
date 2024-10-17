#include "Script/Library/FIVSNode_Convert.h"

#include "DefaultValueHelper.h"
#include "FicsItReflection.h"
#include "Kernel/FIVSRuntimeContext.h"

void FFIVSNodeStatement_Convert::PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	Context.Push_EvaluatePin(Input);
}

void FFIVSNodeStatement_Convert::ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const {
	const FFIRAnyValue* inputVal = Context.TryGetRValue(Input);
	Context.SetValue(Output, FIRCastValue(*inputVal, ToType));
}

void UFIVSNode_Convert::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (EFIRValueType ConvertFromType : TEnumRange<EFIRValueType>()) {
		// Input FIN_ANY is excluded from conversion because it may fail or not and needs its own node
		if (ConvertFromType == FIR_ARRAY || ConvertFromType == FIR_NIL || ConvertFromType == FIR_ANY || ConvertFromType == FIR_STRUCT) continue;
		for (EFIRValueType ConvertToType : TEnumRange<EFIRValueType>()) {
			// Output FIN_ANY is excluded from conversion because it can be casted implicitly and expanded network type allows everything to implicitly convert to any
			if (ConvertToType == FIR_ARRAY || ConvertToType == FIR_NIL || ConvertToType == FIR_STRUCT || ConvertToType == FIR_ANY) continue;
			if (ConvertFromType == ConvertToType) continue;
			// Checks if default conversion of input and output type works
			if (FIRCastValue(FFIRAnyValue::DefaultValue(ConvertFromType), ConvertToType).GetType() != FIR_NIL) {
				FFIVSNodeAction Action;
				Action.NodeType = UFIVSNode_Convert::StaticClass();
				Action.Title = FText::FromString(TEXT("Convert ") + FIRGetNetworkValueTypeName(ConvertFromType) + TEXT(" to ") + FIRGetNetworkValueTypeName(ConvertToType));
				Action.SearchableText = Action.Title;
				Action.Category = FText::FromString(TEXT("Conversions"));
				if (ConvertFromType == FIR_TRACE || ConvertFromType == FIR_OBJ || ConvertFromType == FIR_CLASS) // TODO: Maybe have to work a bit more on the expanded types
					Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIRExtendedValueType(ConvertFromType, FFicsItReflectionModule::Get().FindClass(UObject::StaticClass()))));
				else
					Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIVSPinDataType(ConvertFromType)));
				if (ConvertToType == FIR_TRACE || ConvertToType == FIR_OBJ || ConvertToType == FIR_CLASS) // TODO: Maybe have to work a bit more on the expanded types
					Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIRExtendedValueType(ConvertToType, FFicsItReflectionModule::Get().FindClass(UObject::StaticClass()))));
				else
					Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(ConvertToType)));
				Action.OnExecute.BindLambda([ConvertFromType, ConvertToType](UFIVSNode* Node) {
					Cast<UFIVSNode_Convert>(Node)->SetConversion(ConvertFromType, ConvertToType);
				});
				Actions.Add(Action);
			}
		}
	}
}

void UFIVSNode_Convert::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetStringField(TEXT("From"), FString::FromInt(FromType));
	Properties->SetStringField(TEXT("To"), FString::FromInt(ToType));
}

void UFIVSNode_Convert::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	if (!Properties) return;

	FDefaultValueHelper::ParseInt(Properties->GetStringField(TEXT("From")), (int&)FromType);
	FDefaultValueHelper::ParseInt(Properties->GetStringField(TEXT("To")), (int&)ToType);

	SetConversion(FromType, ToType);
}

void UFIVSNode_Convert::SetConversion(EFIRValueType InFromType, EFIRValueType InToType) {
	FromType = InFromType;
	ToType = InToType;

	DisplayName = FText::FromString(TEXT("Convert ") + FIRGetNetworkValueTypeName(FromType) + TEXT(" to ") + FIRGetNetworkValueTypeName(ToType));

	DeletePin(Input);
	DeletePin(Output);

	if (FromType == FIR_TRACE || FromType == FIR_OBJ || FromType == FIR_CLASS) // TODO: Maybe have to work a bit more on the expanded types
		Input = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Input"), FText::FromString(TEXT("Input")), FFIRExtendedValueType(FromType, FFicsItReflectionModule::Get().FindClass(UObject::StaticClass())));
	else
		Input = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Input"), FText::FromString(TEXT("Input")), FFIRExtendedValueType(FromType));
	if (ToType == FIR_TRACE || ToType == FIR_OBJ || ToType == FIR_CLASS) // TODO: Maybe have to work a bit more on the expanded types
		Output = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Output"), FText::FromString(TEXT("Output")), FFIRExtendedValueType(ToType, FFicsItReflectionModule::Get().FindClass(UObject::StaticClass())));
	else
		Output = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Output"), FText::FromString(TEXT("Output")), FFIRExtendedValueType(ToType));
}
