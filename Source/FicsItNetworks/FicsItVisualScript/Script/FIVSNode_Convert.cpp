#include "FIVSNode_Convert.h"

#include "FicsItNetworks/Reflection/FINReflection.h"

TArray<FFIVSNodeAction> UFIVSNode_Convert::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;
	for (EFINNetworkValueType ConvertFromType : TEnumRange<EFINNetworkValueType>()) {
		// Input FIN_ANY is excluded from conversion because it may fail or not and needs its own node
		if (ConvertFromType == FIN_ARRAY || ConvertFromType == FIN_NIL || ConvertFromType == FIN_ANY || ConvertFromType == FIN_STRUCT) continue;
		for (EFINNetworkValueType ConvertToType : TEnumRange<EFINNetworkValueType>()) {
			// Output FIN_ANY is excluded from conversion because it can be casted implicitly and expanded network type allows everything to implicitly convert to any
			if (ConvertToType == FIN_ARRAY || ConvertToType == FIN_NIL || ConvertToType == FIN_STRUCT || ConvertToType == FIN_ANY) continue;
			if (ConvertFromType == ConvertToType) continue;
			// Checks if default conversion of input and output type works
			if (FINCastNetworkValue(FFINAnyNetworkValue(ConvertFromType), ConvertToType).GetType() != FIN_NIL) {
				FFIVSNodeAction Action;
				Action.NodeType = UFIVSNode_Convert::StaticClass();
				Action.Title = FText::FromString(TEXT("Convert ") + FINGetNetworkValueTypeName(ConvertFromType) + TEXT(" to ") + FINGetNetworkValueTypeName(ConvertToType));
				Action.SearchableText = Action.Title;
				Action.Category = FText::FromString(TEXT("Conversions"));
				if (ConvertFromType == FIN_TRACE || ConvertFromType == FIN_OBJ || ConvertFromType == FIN_CLASS) // TODO: Maybe have to work a bit more on the expanded types
					Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(ConvertFromType, FFINReflection::Get()->FindClass(UObject::StaticClass()))));
				else
					Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, ConvertFromType));
				if (ConvertToType == FIN_TRACE || ConvertToType == FIN_OBJ || ConvertToType == FIN_CLASS) // TODO: Maybe have to work a bit more on the expanded types
					Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFINExpandedNetworkValueType(ConvertToType, FFINReflection::Get()->FindClass(UObject::StaticClass()))));
				else
					Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, ConvertToType));
				Action.OnExecute.BindLambda([ConvertFromType, ConvertToType](UFIVSNode* Node) {
					Cast<UFIVSNode_Convert>(Node)->FromType = ConvertFromType;
					Cast<UFIVSNode_Convert>(Node)->ToType = ConvertToType;;
				});
				Actions.Add(Action);
			}
		}
	}
	return Actions;
}

void UFIVSNode_Convert::SerializeNodeProperties(FFIVSNodeProperties& Properties) const {
	Properties.Properties.Add(TEXT("From"), FString::FromInt(FromType));
	Properties.Properties.Add(TEXT("To"), FString::FromInt(ToType));
}

void UFIVSNode_Convert::DeserializeNodeProperties(const FFIVSNodeProperties& Properties) {
	FDefaultValueHelper::ParseInt(Properties.Properties[TEXT("From")], (int&)FromType);
	FDefaultValueHelper::ParseInt(Properties.Properties[TEXT("To")], (int&)ToType);
}

void UFIVSNode_Convert::InitPins() {
	if (FromType == FIN_TRACE || FromType == FIN_OBJ || FromType == FIN_CLASS) // TODO: Maybe have to work a bit more on the expanded types
		Input = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("Input")), FFINExpandedNetworkValueType(FromType, FFINReflection::Get()->FindClass(UObject::StaticClass())));
	else
		Input = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("Input")), FFINExpandedNetworkValueType(FromType));
	if (ToType == FIN_TRACE || ToType == FIN_OBJ || ToType == FIN_CLASS) // TODO: Maybe have to work a bit more on the expanded types
		Output = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString(TEXT("Output")), FFINExpandedNetworkValueType(ToType, FFINReflection::Get()->FindClass(UObject::StaticClass())));
	else
		Output = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString(TEXT("Output")), FFINExpandedNetworkValueType(ToType));
}

TArray<UFIVSPin*> UFIVSNode_Convert::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{Input};
}

UFIVSPin* UFIVSNode_Convert::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFINAnyNetworkValue InputVal = *Context.GetValue(Input);
	Context.SetValue(Output, FINCastNetworkValue(InputVal, ToType));
	return nullptr;
}
