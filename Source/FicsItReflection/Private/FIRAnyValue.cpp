#include "FIRAnyValue.h"

#include "Engine/World.h"

bool FFIRAnyValue::Serialize(FStructuredArchive::FSlot Slot) {
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	if (Slot.GetUnderlyingArchive().IsLoading()) {
		switch (Type) {
		case FIR_STR:
			delete Data.STRING;
			break;
		case FIR_OBJ:
			delete Data.OBJECT;
			break;
		case FIR_TRACE:
			delete Data.TRACE;
			break;
		case FIR_STRUCT:
			delete Data.STRUCT;
			break;
		case FIR_ARRAY:
			delete Data.ARRAY;
			break;
		case FIR_ANY:
			delete Data.ANY;
			break;
		default:
			break;
		}
	}
	Record.EnterField(TEXT("Type")) << Type;
	if (Slot.GetUnderlyingArchive().IsLoading()) {
		switch (Type) {
		case FIR_STR:
			Data.STRING = new FIRStr();
			break;
		case FIR_OBJ:
			Data.OBJECT = new FIRObj();
			break;
		case FIR_TRACE:
			Data.TRACE = new FIRTrace();
			break;
		case FIR_STRUCT:
			Data.STRUCT = new FIRStruct();
			break;
		case FIR_ARRAY:
			Data.ARRAY = new FIRArray();
			break;
		case FIR_ANY:
			Data.ANY = new FIRAny();
		default:
			break;
		}
	}

	switch (Type) {
	case FIR_INT:
		Record.EnterField(TEXT("FIR_INT")) << Data.INT;
		break;
	case FIR_FLOAT:
		Record.EnterField(TEXT("FIR_FLOAT")) << Data.FLOAT;
		break;
	case FIR_BOOL:
		Record.EnterField(TEXT("FIR_BOOL")) << Data.BOOL;
		break;
	case FIR_STR:
		Record.EnterField(TEXT("FIR_STR")) << *Data.STRING;
		break;
	case FIR_OBJ:
		Record.EnterField(TEXT("FIR_OBJ")) << *Data.OBJECT;
		break;
	case FIR_CLASS:
		Record.EnterField(TEXT("FIR_CLASS")) << Data.CLASS;
		break;
	case FIR_TRACE:
		Record.EnterField(TEXT("FIR_TRACE")) << *Data.TRACE;
		break;
	case FIR_STRUCT:
		Record.EnterField(TEXT("FIR_STRUCT")) << *Data.STRUCT;
		break;
	case FIR_ARRAY:
		Record.EnterField(TEXT("FIR_ARRAY")) << *Data.ARRAY;
		break;
	case FIR_ANY:
		Record.EnterField(TEXT("FIR_ANY")) << *Data.ANY;
		break;
	default:
		break;
	}
	return true;
}
