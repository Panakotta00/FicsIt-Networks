#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Network/NetworkTrace.h"
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

inline bool FFINNetworkTrace::Serialize(FArchive& Ar) {
	if (Ar.IsSaveGame()) {
		UObject* ptr = trace.getUnderlyingPtr().Get();
		Ar << ptr;
		trace.obj = ptr;
	}

	return true;
}

inline FArchive& operator<<(FArchive& Ar, FFINNetworkTrace& trace) {
	trace.Serialize(Ar);
	return Ar;
}

FORCEINLINE uint32 GetTypeHash(const FFINNetworkTrace& trace) {
	return GetTypeHash(trace.trace.getUnderlyingPtr());
}