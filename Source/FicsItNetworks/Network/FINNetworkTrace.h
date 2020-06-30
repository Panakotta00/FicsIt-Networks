#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Network/NetworkTrace.h"
#include "util/Logging.h"

#include "FINNetworkTrace.generated.h"

/**
 * Tracks the access of a object through the network.
 * Allows a later check if the object is still reachable
 */
USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINNetworkTrace {
	GENERATED_BODY()

	friend uint32 GetTypeHash(const FFINNetworkTrace&);

private:
	FicsItKernel::Network::NetworkTrace trace;

public:
	FFINNetworkTrace() : trace(nullptr) {}
	explicit FFINNetworkTrace(UObject* obj) : trace(obj) {}
	FFINNetworkTrace(const FicsItKernel::Network::NetworkTrace& trace) : trace(trace) {}

	bool Serialize(FArchive& Ar);

	operator FicsItKernel::Network::NetworkTrace() {
		return trace;
	}

	FFINNetworkTrace operator()(UObject* obj) const {
		return trace(obj);
	}

	FFINNetworkTrace operator/(UObject* obj) const {
		return trace / obj;
	}

	UObject* operator*() const {
		return *trace;
	}

	bool operator==(const FFINNetworkTrace& other) const {
		return trace == other.trace;
	}

	FicsItKernel::Network::NetworkTrace& getTrace() {
		return trace;
	}
};

inline FArchive& operator<<(FArchive& Ar, FFINNetworkTrace& trace) {
	trace.Serialize(Ar);
	return Ar;
}

inline bool FFINNetworkTrace::Serialize(FArchive& Ar) {
	SML::Logging::error("FIN Serialize!");
	if (Ar.IsSaveGame()) {
		bool valid = trace.isValid();
		Ar << valid;
		if (valid) {
			// obj ptr
			UObject* ptr = trace.getUnderlyingPtr().Get();
			Ar << ptr;
			trace.obj = ptr;
	
			// prev trace
			bool hasPrev = trace.prev;
			Ar << hasPrev;
			if (hasPrev) {
				FFINNetworkTrace prev;
				if (Ar.IsSaving()) prev = *trace.prev;
				Ar << prev;
				if (Ar.IsLoading()) {
					if (trace.prev) delete trace.prev;
					trace.prev = new FicsItKernel::Network::NetworkTrace(prev);
				}
			}

			// step
			bool hasStep = trace.step.get();
			Ar << hasStep;
			if (hasStep) {
				FString save;
				if (Ar.IsSaving()) save = FicsItKernel::Network::NetworkTrace::inverseTraceStepRegistry[trace.step].c_str();
				Ar << save;
				if (Ar.IsLoading()) trace.step = FicsItKernel::Network::NetworkTrace::traceStepRegistry[TCHAR_TO_UTF8(*save)];
			}
		}
	}
	
	return true;
}

FORCEINLINE uint32 GetTypeHash(const FFINNetworkTrace& trace) {
	return GetTypeHash(trace.trace.getUnderlyingPtr());
}