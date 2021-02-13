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

	bool Serialize(FArchive& Ar) {
		if (Ar.IsLoading() && Type == TRACE) delete Trace;
		Ar << *reinterpret_cast<uint8*>(&Type);
		switch (Type) {
		case NONE:
			break;
		case GENERIC:
			Type = NONE;
			break;
		case OBJECT:
			Ar << Obj;
			break;
		case TRACE:
			if (Ar.IsLoading()) Trace = new FFINNetworkTrace();
			Ar << *Trace;
			break;
		default: ;
		}
		
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
	Ctx.Serialize(Ar);
	return Ar;
}

template<>
struct TStructOpsTypeTraits<FFINExecutionContext> : TStructOpsTypeTraitsBase2<FFINExecutionContext> {
	enum {
		WithSerializer = true,
    };
};
