#include "FIVSUtils.h"

#include "FicsItReflection.h"
#include "FINNetworkComponent.h"
#include "JsonObject.h"

FString FIRObjectToString(UObject* InObj) {
	if (!InObj) return TEXT("Nil");
	FString Text = FFicsItReflectionModule::Get().FindClass(InObj->GetClass())->GetInternalName();
	if (InObj->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
		Text += TEXT(" ") + IFINNetworkComponent::Execute_GetID(InObj).ToString();
		FString Nick = IFINNetworkComponent::Execute_GetNick(InObj);
		if (Nick.Len() > 0) {
			Text += TEXT(" '") + Nick + TEXT("'");
		}
	}
	return Text;
}

FString FIRClassToString(UClass* InClass) {
	if (!InClass) return TEXT("Nil");
	return FFicsItReflectionModule::Get().FindClass(InClass)->GetInternalName() + TEXT("-Type");
}

TSharedRef<FJsonObject> FIRExtendedValueTypeToJsonObject(const FFIRExtendedValueType& Type) {
	auto obj = MakeShared<FJsonObject>();
	obj->SetStringField(TEXT("type"), EnumToString(Type.GetType()));
	if (Type.HasRefSubType()) {
		obj->SetStringField(TEXT("refSubType"), Type.GetRefSubType()->GetPathName());
	}
	if (Type.HasSubType()) {
		obj->SetObjectField(TEXT("subType"), FIRExtendedValueTypeToJsonObject(Type.GetSubType()));
	}
	return obj;
}
bool FIRExtendedValueTypeFromJsonObject(TSharedPtr<FJsonObject> JsonObject, FFIRExtendedValueType& Type) {
	if (!JsonObject) return false;

	FString typeString;
	if (!JsonObject->TryGetStringField(TEXT("type"), typeString)) {
		return false;
	}
	FString refSubTypeString;
	JsonObject->TryGetStringField(TEXT("refSubType"), refSubTypeString);

	const TSharedPtr<FJsonObject>* subType = nullptr;
	JsonObject->TryGetObjectField(TEXT("subType"), subType);
	EFIRValueType type = StringToFIRValueType(typeString).Get(FIR_ANY);
	if (!refSubTypeString.IsEmpty()) {
		Type = FFIRExtendedValueType(type, Cast<UFIRStruct>(FSoftObjectPath(refSubTypeString).TryLoad()));
	} else if (subType) {
		FFIRExtendedValueType subTypeValue;
		FIRExtendedValueTypeFromJsonObject(*subType, subTypeValue);
		Type = FFIRExtendedValueType(type, subTypeValue);
	} else {
		Type = FFIRExtendedValueType(type);
	}
	return true;
}

TSharedRef<FJsonObject> FIVSPinDataTypeToJsonObject(const FFIVSPinDataType& Type) {
	auto obj = FIRExtendedValueTypeToJsonObject(Type);
	obj->SetBoolField(TEXT("reference"), Type.IsReference());
	return obj;
}

bool FIVSPinDataTypeFromJsonObject(TSharedPtr<FJsonObject> JsonObject, FFIVSPinDataType& Type) {
	if (!JsonObject) return false;

	if (!FIRExtendedValueTypeFromJsonObject(JsonObject, Type)) {
		return false;
	}
	bool isReference;
	if (JsonObject->TryGetBoolField(TEXT("reference"), isReference)) {
		Type.bReference = isReference;
	}
	return true;
}
