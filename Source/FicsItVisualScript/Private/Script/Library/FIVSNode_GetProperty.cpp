#include "Script/Library/FIVSNode_GetProperty.h"

#include "FicsItReflection.h"
#include "FINNetworkUtils.h"
#include "JsonObject.h"

void UFIVSNode_GetProperty::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (TPair<UClass*, UFIRClass*> Class : FFicsItReflectionModule::Get().GetClasses()) {
		for (UFIRProperty* GetProperty : Class.Value->GetProperties(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_GetProperty::StaticClass();
			if (GetProperty->GetPropertyFlags() & FIR_Prop_ClassProp) {
				Action.Title = FText::FromString(TEXT("Get ") + GetProperty->GetDisplayName().ToString() + TEXT(" (Class)"));
			} else {
				Action.Title = FText::FromString(TEXT("Get ") + GetProperty->GetDisplayName().ToString());
			}
			Action.SearchableText = Action.Title;
			Action.Category = FText::FromString(Class.Value->GetDisplayName().ToString() + TEXT("|Properties"));
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFIRExtendedValueType(GetProperty->GetPropertyFlags() & FIR_Prop_ClassProp ? FIR_CLASS : FIR_TRACE, Class.Value)));
			FFIVSFullPinType PinType(GetProperty);
			PinType.PinType |= FIVS_PIN_OUTPUT;
			Action.Pins.Add(PinType);
			Action.OnExecute.BindLambda([GetProperty](UFIVSNode* Node) {
				Cast<UFIVSNode_GetProperty>(Node)->SetProperty(GetProperty);
			});
			Actions.Add(Action);
		}
	}
}

void UFIVSNode_GetProperty::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	Properties->SetStringField(TEXT("Property"), Property->GetPathName());
}

void UFIVSNode_GetProperty::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	SetProperty(Cast<UFIRProperty>(FSoftObjectPath(Properties->GetStringField(TEXT("Property"))).TryLoad()));
}

void UFIVSNode_GetProperty::CompileNodeToLua(FFIVSLuaCompilerContext& Context) {
	if (IsValid(Property)) {
		FString expInput = Context.GetRValueExpression(InstanceIn);
		FString propName = Property->GetInternalName();
		Context.AddExpression(DataOut, FString::Printf(TEXT("%s.%s"), *expInput, *propName));
	} else {
		Context.AddCompileError(FFIVSCompileError(this, FText::FromString(TEXT("Unknown Property"))));
	}
}

void UFIVSNode_GetProperty::SetProperty(UFIRProperty* InProperty) {
	Property = InProperty;

	if (Property) {
		if (Property->GetPropertyFlags() & FIR_Prop_ClassProp) {
			DisplayName = FText::FromString(TEXT("Get ") + Property->GetInternalName() + TEXT(" (Class)"));
		} else {
			DisplayName = FText::FromString(TEXT("Get ") + Property->GetInternalName());
		}
	}

	DeletePin(InstanceIn);
	DeletePin(DataOut);

	if (Property) {
		InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, TEXT("Instance"), FText::FromString("Instance"), FFIVSPinDataType(Property->GetPropertyFlags() & FIR_Prop_ClassProp ? FIR_CLASS : FIR_TRACE, Cast<UFIRClass>(Property->GetOuter())));
		FFIVSPinDataType Type(Property);
		if (Type.GetType() == FIR_OBJ) Type = FFIVSPinDataType(FIR_TRACE, Type.GetRefSubType());
		DataOut = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Value"), FText::FromString("Value"), Type);
	}
}
