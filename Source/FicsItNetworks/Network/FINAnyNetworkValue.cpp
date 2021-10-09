#include "FINAnyNetworkValue.h"

#include "FicsItNetworks/Reflection/FINReflection.h"

bool FFINAnyNetworkValue::Serialize(FArchive& Ar) {
	if (Ar.IsLoading()) {
		switch (Type) {
		case FIN_STR:
			delete Data.STRING;
			break;
		case FIN_OBJ:
			delete Data.OBJECT;
			break;
		case FIN_TRACE:
			delete Data.TRACE;
			break;
		case FIN_STRUCT:
			delete Data.STRUCT;
			break;
		case FIN_ARRAY:
			delete Data.ARRAY;
			break;
		case FIN_ANY:
			delete Data.ANY;
			break;
		default:
			break;
		}
	}
	Ar << Type;
	if (Ar.IsLoading()) {
		switch (Type) {
		case FIN_STR:
			Data.STRING = new FINStr();
			break;
		case FIN_OBJ:
			Data.OBJECT = new FINObj();
			break;
		case FIN_TRACE:
			Data.TRACE = new FINTrace();
			break;
		case FIN_STRUCT:
			Data.STRUCT = new FINStruct();
			break;
		case FIN_ARRAY:
			Data.ARRAY = new FINArray();
			break;
		case FIN_ANY:
			Data.ANY = new FINAny();
		default:
			break;
		}
	}

	switch (Type) {
	case FIN_INT:
		Ar << Data.INT;
		break;
	case FIN_FLOAT:
		Ar << Data.FLOAT;
		break;
	case FIN_BOOL:
		Ar << Data.BOOL;
		break;
	case FIN_STR:
		Ar << *Data.STRING;
		break;
	case FIN_OBJ:
		Ar << *Data.OBJECT;
		break;
	case FIN_CLASS:
		Ar << Data.CLASS;
		break;
	case FIN_TRACE:
		Ar << *Data.TRACE;
		break;
	case FIN_STRUCT:
		Ar << *Data.STRUCT;
		break;
	case FIN_ARRAY:
		Ar << *Data.ARRAY;
		break;
	case FIN_ANY:
		Ar << *Data.ANY;
		break;
	default:
		break;
	}
	return true;
}

FString FINObjectToString(UObject* InObj) {
	if (!InObj) return TEXT("Nil");
	FString Text = FFINReflection::Get()->FindClass(InObj->GetClass())->GetInternalName();
	if (InObj->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
		Text += TEXT(" ") + IFINNetworkComponent::Execute_GetID(InObj).ToString();
		FString Nick = IFINNetworkComponent::Execute_GetNick(InObj);
		if (Nick.Len() > 0) {
			Text += TEXT(" '") + Nick + TEXT("'");
		}
	}
	return Text;
}

FString FINClassToString(UClass* InClass) {
	if (!InClass) return TEXT("Nil");
	return FFINReflection::Get()->FindClass(InClass)->GetInternalName() + TEXT("-Type");
}
