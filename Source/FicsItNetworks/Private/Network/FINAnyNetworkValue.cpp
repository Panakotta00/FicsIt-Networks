#include "Network/FINAnyNetworkValue.h"

#include "Engine/World.h"
#include "Utils/FINUtils.h"

bool FFINAnyNetworkValue::Serialize(FStructuredArchive::FSlot Slot) {
	FVersion version = UFINUtils::GetFINSaveVersion(GWorld);
	if (FVersion(0, 3, 19).Compare(version) == 1) return false;
	
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	if (Slot.GetUnderlyingArchive().IsLoading()) {
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
	Record.EnterField(SA_FIELD_NAME(TEXT("Type"))) << Type;
	if (Slot.GetUnderlyingArchive().IsLoading()) {
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
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_INT"))) << Data.INT;
		break;
	case FIN_FLOAT:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_FLOAT"))) << Data.FLOAT;
		break;
	case FIN_BOOL:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_BOOL"))) << Data.BOOL;
		break;
	case FIN_STR:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_STR"))) << *Data.STRING;
		break;
	case FIN_OBJ:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_OBJ"))) << *Data.OBJECT;
		break;
	case FIN_CLASS:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_CLASS"))) << Data.CLASS;
		break;
	case FIN_TRACE:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_TRACE"))) << *Data.TRACE;
		break;
	case FIN_STRUCT:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_STRUCT"))) << *Data.STRUCT;
		break;
	case FIN_ARRAY:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_ARRAY"))) << *Data.ARRAY;
		break;
	case FIN_ANY:
		Record.EnterField(SA_FIELD_NAME(TEXT("FIN_ANY"))) << *Data.ANY;
		break;
	default:
		break;
	}
	return true;
}
