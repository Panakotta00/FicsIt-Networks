#pragma once

#include "FINSignal.h"
#include "Network/FINNetworkTrace.h"
#include "FINSmartSignal.generated.h"

struct VariaDicSignalElem {
	enum Type {
		Int,
		Float,
		Bool,
		String,
		Object,
		Trace,
		Class,
		Item,
		ItemAmount,
		Stack
	};

	VariaDicSignalElem(int e);

	VariaDicSignalElem(float e);

	VariaDicSignalElem(bool e);

	VariaDicSignalElem(FString e);

	VariaDicSignalElem(UObject* e);

	VariaDicSignalElem(const FFINNetworkTrace& e);

	VariaDicSignalElem(const FInventoryItem& e);

	VariaDicSignalElem(const FItemAmount& e);

	VariaDicSignalElem(const FInventoryStack& e);

	VariaDicSignalElem(const VariaDicSignalElem& other);

	VariaDicSignalElem(FArchive& Ar);

	VariaDicSignalElem& operator=(const VariaDicSignalElem& other);

	~VariaDicSignalElem();

	Type getType() const {
		return type;
	}

	int getInt() const {
		return data.INT;
	}

	float getFloat() const {
		return data.FLOAT;
	}

	bool getBool() const {
		return data.BOOL;
	}

	FString getString() const {
		return *data.STRING;
	}

	UObject* getObject() const {
		return data.OBJECT;
	}

	UClass* getUClass() const {
		return Cast<UClass>(data.OBJECT);
	}

	const FInventoryItem* getItem() const {
		return data.ITEM;
	}

	const FItemAmount* getItemAmount() const {
		return data.ITEM_AMOUNT;
	}

	const FInventoryStack* getStack() const {
		return data.STACK;
	}

	void serialize(FArchive& Ar);

private:
	Type type;
	union {
		int					INT;
		float				FLOAT;
		bool				BOOL;
		FString*			STRING;
		UObject*			OBJECT;
		FFINNetworkTrace*	TRACE;
		FInventoryItem*		ITEM;
		FItemAmount*		ITEM_AMOUNT;
		FInventoryStack*	STACK;
	} data;
};

/**
 * A signal container which uses a c++ variadic parameters list
 * to store the signal parameters.
 */
USTRUCT()
struct FFINSmartSignal : public FFINSignal {
	GENERATED_BODY()
	
protected:
	TArray<VariaDicSignalElem> args;

public:
	FFINSmartSignal();
	FFINSmartSignal(FString name, const TArray<VariaDicSignalElem>& args) : FFINSignal(name), args(args) {}
	
	template<typename... Ts>
	FFINSmartSignal(FString signalName, Ts&&... args) : FFINSmartSignal(signalName, {VariaDicSignalElem(args)...}) {}

	FFINSmartSignal(FArchive& Ar);
	
	bool Serialize(FArchive& Ar);
	
	// Begin FFINSignal
	virtual int operator>>(FFINValueReader& reader) const override;
	virtual UScriptStruct* GetStruct() const override;
	// End FFINSignal
};

