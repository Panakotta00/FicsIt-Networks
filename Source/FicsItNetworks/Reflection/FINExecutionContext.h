#pragma once

#include "Network/FINNetworkTrace.h"

class FFINExecutionContext {
public:
	enum Type {
		GENERIC,
        OBJECT,
        TRACE
    };

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
	
private:
	Type Type;
	
	union {
		void* Generic;
		UObject* Obj;
		FFINNetworkTrace* Trace;
	};
};
