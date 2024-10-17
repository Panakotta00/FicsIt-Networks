#pragma once

#include "CoreMinimal.h"
#include "FIRTrace.h"
#include "FIRExecutionContext.generated.h"

USTRUCT()
struct FICSITREFLECTION_API FFIRExecutionContext {
	GENERATED_BODY()

public:
	enum Type : uint8 {
		NONE = 0,
		GENERIC = 1,
        OBJECT = 2,
        TRACE = 3
    };

	FORCEINLINE FFIRExecutionContext() : Obj(nullptr) {
		Type = NONE;
	}
	
	FORCEINLINE FFIRExecutionContext(void* Generic) : Generic(Generic) {
		Type = GENERIC;
	}

	FORCEINLINE FFIRExecutionContext(UObject* Obj) : Obj(Obj) {
		Type = OBJECT;
	}

	FORCEINLINE FFIRExecutionContext(const FFIRTrace& InTrace) {
		Type = TRACE;
		Trace = new FFIRTrace(InTrace);
	}

	FORCEINLINE FFIRExecutionContext(const FFIRExecutionContext& Other) {
		Type = GENERIC;
		*this = Other;
	}

	FORCEINLINE ~FFIRExecutionContext() {
		switch (Type) {
		case TRACE:
			delete Trace;
			break;
		default: ;
		}
	}
	
	FORCEINLINE FFIRExecutionContext& operator=(const FFIRExecutionContext& Other) {
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
			Trace = new FFIRTrace(*Other.Trace);
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

	FORCEINLINE FFIRTrace GetTrace() const {
		switch (Type) {
		case OBJECT:
			return FFIRTrace(Obj);
		case TRACE:
			return *Trace;
		default: ;
		}
		return FFIRTrace();
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
			if (Type != TRACE) Trace = new FFIRTrace();
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
		FFIRTrace* Trace;
	};
};

inline FArchive& operator<<(FArchive& Ar, FFIRExecutionContext& Ctx) {
	Ctx.Serialize(FStructuredArchiveFromArchive(Ar).GetSlot());
	return Ar;
}

inline void operator<<(FStructuredArchive::FSlot Slot, FFIRExecutionContext& Ctx) {
	Ctx.Serialize(Slot);
}

template<>
struct TStructOpsTypeTraits<FFIRExecutionContext> : TStructOpsTypeTraitsBase2<FFIRExecutionContext> {
	enum {
		WithStructuredSerializer = true,
    };
};
