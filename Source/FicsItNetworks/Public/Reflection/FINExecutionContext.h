#pragma once

#include "Network/FINNetworkTrace.h"
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

	FORCEINLINE FFINExecutionContext() : Obj(nullptr) {
		Type = NONE;
	}
	
	FORCEINLINE FFINExecutionContext(void* Generic) : Generic(Generic) {
		Type = GENERIC;
	}

	FORCEINLINE FFINExecutionContext(UObject* Obj) : Obj(Obj) {
		Type = OBJECT;
	}

	FORCEINLINE FFINExecutionContext(const FFINNetworkTrace& InTrace) {
		Type = TRACE;
		Trace = new FFINNetworkTrace(InTrace);
	}

	FORCEINLINE FFINExecutionContext(const FFINExecutionContext& Other) {
		Type = GENERIC;
		*this = Other;
	}

	FORCEINLINE ~FFINExecutionContext() {
		switch (Type) {
		case TRACE:
			delete Trace;
			break;
		default: ;
		}
	}
	
	FORCEINLINE FFINExecutionContext& operator=(const FFINExecutionContext& Other) {
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
		default: ;
		}
		
		return *this;
	}

	FORCEINLINE Type GetType() const {
		return Type;
	}

	FORCEINLINE void* GetGeneric() const {
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

	FORCEINLINE UObject* GetObject() const {
		switch (Type) {
		case OBJECT:
			return Obj;
		case TRACE:
			return **Trace;
		default: ;
		}
		return nullptr;
	}

	FORCEINLINE FFINNetworkTrace GetTrace() const {
		switch (Type) {
		case OBJECT:
			return FFINNetworkTrace(Obj);
		case TRACE:
			return *Trace;
		default: ;
		}
		return FFINNetworkTrace();
	}

	FORCEINLINE bool IsValid() const {
		switch (Type) {
		case GENERIC:
			return Generic != nullptr;
		case OBJECT:
			return Obj != nullptr;
		case TRACE:
			return Trace->IsValid();
		default: ;
		}
		return false;
	}

	FORCEINLINE bool Serialize(FStructuredArchive::FSlot Slot) {
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
