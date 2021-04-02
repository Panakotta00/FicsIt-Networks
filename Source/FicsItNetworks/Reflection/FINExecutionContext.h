#pragma once

#include "FicsItNetworks/Network/FINNetworkTrace.h"
#include "FINExecutionContext.generated.h"

USTRUCT()
struct FICSITNETWORKS_API FFINExecutionContext {
	GENERATED_BODY()

public:
	enum Type : uint8 {
		NONE = 0,
		GENERIC = 1,
        OBJECT = 2,
        TRACE = 3
    };

	FFINExecutionContext() : Obj(nullptr) {
		Type = NONE;
	}
	
	FFINExecutionContext(void* Generic) : Generic(Generic) {
		Type = GENERIC;
	}

	FFINExecutionContext(UObject* Obj) : Obj(Obj) {
		Type = OBJECT;
	}

	FFINExecutionContext(const FFINNetworkTrace& InTrace) {
		Type = TRACE;
		Trace = new FFINNetworkTrace(InTrace);
	}

	FFINExecutionContext(const FFINExecutionContext& Other) {
		Type = GENERIC;
		*this = Other;
	}

	~FFINExecutionContext() {
		switch (Type) {
		case TRACE:
			delete Trace;
			break;
		default: ;
		}
	}
	
	FFINExecutionContext& operator=(const FFINExecutionContext& Other) {
		switch (Type) {
		case TRACE:
			delete Trace;
			break;
		default: ;
		}

		Type = Other.Type;
		switch (Type) {
		case GENERIC:
			Generic = Other.Generic;
			break;
		case OBJECT:
			Obj = Other.Obj;
			break;
		case TRACE:
			Trace = new FFINNetworkTrace(*Other.Trace);
			break;
		}
		
		return *this;
	}

	Type GetType() const {
		return Type;
	}

	void* GetGeneric() const {
		switch (Type) {
		case GENERIC:
			return Generic;
		case OBJECT:
			return Obj;
		case TRACE:
			return **Trace;
		default: ;
		}
		return nullptr;
	}

	UObject* GetObject() const {
		switch (Type) {
		case OBJECT:
			return Obj;
		case TRACE:
			return **Trace;
		default: ;
		}
		return nullptr;
	}

	FFINNetworkTrace GetTrace() const {
		switch (Type) {
		case OBJECT:
			return FFINNetworkTrace(Obj);
		case TRACE:
			return *Trace;
		default: ;
		}
		return FFINNetworkTrace();
	}

	bool Serialize(FStructuredArchive::FSlot Slot) {
		FStructuredArchive::FRecord Record = Slot.EnterRecord();
		TOptional<FStructuredArchive::FSlot> TraceField = Record.TryEnterField(SA_FIELD_NAME(TEXT("Trace")), Type == TRACE);
		if (TraceField.IsSet()) {
			if (Type != TRACE) Trace = new FFINNetworkTrace();
			Type = TRACE;
			Trace->Serialize(TraceField.GetValue());
			return true;
		} else if (Type == TRACE) {
			delete Trace;
		}

		TOptional<FStructuredArchive::FSlot> ObjectField = Record.TryEnterField(SA_FIELD_NAME(TEXT("Object")), Type == OBJECT);
		if (ObjectField.IsSet()) {
			Type = OBJECT;
			ObjectField.GetValue() << Obj;
			return true;
		}

		if (Record.GetUnderlyingArchive().IsLoading()) Type = NONE;
		return true;
	}
	
private:
	Type Type;
	
	union {
		void* Generic;
		UObject* Obj;
		FFINNetworkTrace* Trace;
	};
};

inline FArchive& operator<<(FArchive& Ar, FFINExecutionContext& Ctx) {
	Ctx.Serialize(FStructuredArchiveFromArchive(Ar).GetSlot());
	return Ar;
}

inline void operator<<(FStructuredArchive::FSlot Slot, FFINExecutionContext& Ctx) {
	Ctx.Serialize(Slot);
}

template<>
struct TStructOpsTypeTraits<FFINExecutionContext> : TStructOpsTypeTraitsBase2<FFINExecutionContext> {
	enum {
		WithStructuredSerializer = true,
    };
};
