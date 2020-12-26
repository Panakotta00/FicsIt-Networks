#include "FINAnyNetworkValue.h"

FFINAnyNetworkValue::FFINAnyNetworkValue() {}

FFINAnyNetworkValue::FFINAnyNetworkValue(FINInt e) {
	Data.INT = e;
	Type = FIN_INT;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(FINFloat e) {
	Data.FLOAT = e;
	Type = FIN_FLOAT;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(FINBool e) {
	Data.BOOL = e;
	Type = FIN_BOOL;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(FINClass e) {
	Data.CLASS = e;
	Type = FIN_CLASS;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(const FINStr& e) {
	Data.STRING = new FINStr(e);
	Type = FIN_STR;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(const FINObj& e) {
	Data.OBJECT = new FINObj(e);
	Type = FIN_OBJ;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(const FINTrace& e) {
	Data.TRACE = new FINTrace(e);
	Type = FIN_TRACE;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(const FINStruct& e) {
	Data.STRUCT = new FINStruct(e);
	Type = FIN_STRUCT;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(const FINArray& e) {
	Data.ARRAY = new FINArray(e);
	Type = FIN_ARRAY;
}

FFINAnyNetworkValue::FFINAnyNetworkValue(const FFINAnyNetworkValue& other) {
	*this = other;
}

FFINAnyNetworkValue& FFINAnyNetworkValue::operator=(const FFINAnyNetworkValue& other) {
	this->~FFINAnyNetworkValue();
	Type = other.Type;
	switch (Type) {
	case FIN_STR:
		Data.STRING = new FINStr(*other.Data.STRING);
		break;
	case FIN_OBJ:
		Data.OBJECT = new FINObj(*other.Data.OBJECT);
		break;
	case FIN_TRACE:
		Data.TRACE = new FINTrace(*other.Data.TRACE);
		break;
	case FIN_STRUCT:
		Data.STRUCT = new FINStruct(*other.Data.STRUCT);
		break;
	case FIN_ARRAY:
		Data.ARRAY = new FINArray(*other.Data.ARRAY);
		break;
	case FIN_ANY:
		Data.ANY = new FINAny(*other.Data.ANY);
		break;
	default:
		Data = other.Data;
		break;
	}
	return *this;
}

FFINAnyNetworkValue::~FFINAnyNetworkValue() {
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
