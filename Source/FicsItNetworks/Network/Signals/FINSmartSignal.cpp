#include "FINSmartSignal.h"

#include "FicsItKernel/FicsItFS/Serial.h"
#include "util/Logging.h"

typedef VariaDicSignalElem::Type Type;

VariaDicSignalElem::VariaDicSignalElem(int e) {
	data.INT = e;
	type = Int;
}

VariaDicSignalElem::VariaDicSignalElem(float e) {
	data.FLOAT = e;
	type = Float;
}

VariaDicSignalElem::VariaDicSignalElem(bool e) {
	data.BOOL = e;
	type = Bool;
}

VariaDicSignalElem::VariaDicSignalElem(FString e) {
	data.STRING = new FString(e);
	type = String;
}

VariaDicSignalElem::VariaDicSignalElem(UObject* e) {
	data.OBJECT = e;
	if (Cast<UClass>(e)) type = Class;
	else type = Object;
}

VariaDicSignalElem::VariaDicSignalElem(const FFINNetworkTrace& e) {
	data.TRACE = new FFINNetworkTrace(e);
	type = Trace;
}

VariaDicSignalElem::VariaDicSignalElem(const FInventoryItem& e) {
	data.ITEM = new FInventoryItem(e);
	type = Item;
}

VariaDicSignalElem::VariaDicSignalElem(const FItemAmount& e) {
	data.ITEM_AMOUNT = new FItemAmount(e);
	type = ItemAmount;
}

VariaDicSignalElem::VariaDicSignalElem(const FInventoryStack& e) {
	data.STACK = new FInventoryStack(e);
	type = Stack;
}

VariaDicSignalElem::VariaDicSignalElem(const VariaDicSignalElem& other) {
	*this = other;
}

VariaDicSignalElem::VariaDicSignalElem(FArchive& Ar) {
	int t;
	Ar << t;
	type = static_cast<Type>(t);

	switch (type) {
	case String:
		data.STRING = new FString();
		break;
	case Trace:
		data.TRACE = new FFINNetworkTrace();
		break;
	case Item:
		data.ITEM = new FInventoryItem();
		break;
	case ItemAmount:
		data.ITEM_AMOUNT = new FItemAmount();
		break;
	case Stack:
		data.STACK = new FInventoryStack();
		break;
	}

	serialize(Ar);
}

VariaDicSignalElem& VariaDicSignalElem::operator=(const VariaDicSignalElem& other) {
	type = other.type;
	switch (type) {
	case String:
		data.STRING = new FString(*other.data.STRING);
		break;
	case Trace:
		data.TRACE = new FFINNetworkTrace(*other.data.TRACE);
		break;
	case Item:
		data.ITEM = new FInventoryItem(*other.data.ITEM);
		break;
	case ItemAmount:
		data.ITEM_AMOUNT = new FItemAmount(*other.data.ITEM_AMOUNT);
		break;
	case Stack:
		data.STACK = new FInventoryStack(*other.data.STACK);
		break;
	default:
		data = other.data;
		break;
	}
	return *this;
}

VariaDicSignalElem::~VariaDicSignalElem() {
	switch (type) {
	case String:
		delete data.STRING;
		break;
	case Trace:
		delete data.TRACE;
		break;
	case Item:
		delete data.ITEM;
		break;
	case ItemAmount:
		delete data.ITEM_AMOUNT;
		break;
	case Stack:
		delete data.STACK;
		break;
	}
}

void VariaDicSignalElem::serialize(FArchive& Ar) {
	if (Ar.IsSaving()) {
		int t = type;
		Ar << t;
	}

	switch (type) {
	case Int:
		Ar << data.INT;
		break;
	case Float:
		Ar << data.FLOAT;
		break;
	case Bool:
		Ar << data.BOOL;
		break;
	case String: {
		Ar << *data.STRING;
		break;
	}
	case Object:
		Ar << data.OBJECT;
		break;
	case Trace: {
		Ar << *data.TRACE;
		break;
	}
	case Class:
		Ar << data.OBJECT;
		break;
	case Item:
		data.ITEM->Serialize(Ar);
		break;
	case ItemAmount:
		Ar << data.ITEM_AMOUNT->Amount;
		Ar << data.ITEM_AMOUNT->ItemClass;
		break;
	case Stack:
		data.STACK->Item.Serialize(Ar);
		Ar << data.STACK->NumItems;
		break;
	}
}

FFINSmartSignal::FFINSmartSignal() : FFINSignal("NoSignal") {}

FFINSmartSignal::FFINSmartSignal(FArchive& Ar) : FFINSignal("") {
	Serialize(Ar);
}

int FFINSmartSignal::operator>>(FFINValueReader& reader) const {
	for (auto& arg : args) {
		switch (arg.getType()) {
		case Type::Int:
			reader << arg.getInt();
			break;
		case Type::Float:
			reader << arg.getFloat();
			break;
		case Type::Bool:
			reader << arg.getBool();
			break;
		case Type::String:
			reader << arg.getString();
			break;
		case Type::Object:
			reader << arg.getObject();
			break;
		case Type::Item:
			reader.WriteAbstract(arg.getItem(), "InventoryItem");
			break;
		case Type::ItemAmount:
			reader.WriteAbstract(arg.getItemAmount(), "ItemAmount");
			break;
		case Type::Stack:
			reader.WriteAbstract(arg.getStack(), "InventoryStack");
			break;
		}
	}
	return args.Num();
}

UScriptStruct* FFINSmartSignal::GetStruct() const {
	return StaticStruct();
}

bool FFINSmartSignal::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
	
	int count = args.Num();
	Ar << count;
	if (Ar.IsSaving()) for (VariaDicSignalElem& arg : args) {
		arg.serialize(Ar);
	}
	if (Ar.IsLoading()) for (int i = 0; i < count; ++i) {
		args.Add(VariaDicSignalElem(Ar));
	}
	
	return true;
}

