#pragma once

#include "CoreMinimal.h"
#include "FINNetworkTrace.generated.h"

/**
* Implements a trace validation step.
* Checks if object B is reachable from object A.
* !IMPORTANT! A and B should be valid pointers and no nullptr.
*/
typedef TFunction<bool(UObject*, UObject*)> FFINTraceStep;

/**
 * Tracks the access of a object through the network.
 * Allows a later check if the object is still reachable
 */
USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINNetworkTrace {
	GENERATED_BODY()

	friend uint32 GetTypeHash(const FFINNetworkTrace&);

private:
	TSharedPtr<FFINNetworkTrace> Prev = nullptr;
	TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> Step = nullptr;

	UPROPERTY()
	UObject* Obj = nullptr;

public:
	static TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> fallbackTraceStep;
	static TArray<TPair<TPair<UClass*, UClass*>, TPair<FString, FFINTraceStep*>>(*)()> toRegister;
	static TMap<FString, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>> traceStepRegistry;
	static TMap<TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>, FString> inverseTraceStepRegistry;
	static TMap<UClass*, TPair<TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>, TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>>> traceStepMap;
	static TMap<UClass*, TPair<TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>, TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>>> interfaceTraceStepMap;

	/**
	 * Trys to find the most suitable trace step of for both given classes
	 */
	static TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> findTraceStep(UClass* A, UClass* B);
	
	FFINNetworkTrace(const FFINNetworkTrace& trace);
	FFINNetworkTrace& operator=(const FFINNetworkTrace& trace);

	explicit FFINNetworkTrace();
	explicit FFINNetworkTrace(UObject* obj);
	~FFINNetworkTrace();

	bool Serialize(FStructuredArchive::FSlot Slot);
	void AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const;

	/**
	 * Creates a copy of this network trace and adds potentially a new optimal trace step
	 * from the current referenced object to the new given one.
	 * Will set the referenced object of the copy to the given object.
	 * If no optimal trace step is found, works like operator()
	 * @return the copied and expanded network trace
	 */
	FFINNetworkTrace operator/(UObject* other) const;
	FFINNetworkTrace append(UObject* other, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> InStep = nullptr) {
		if (!InStep.IsValid()) return *this / other;
		FFINNetworkTrace trace(other);

		UObject* A = Obj;
		if (!::IsValid(A) || !other) return FFINNetworkTrace(nullptr); // if A is not valid, the network trace will always be not invalid
		trace.Prev = MakeShared<FFINNetworkTrace>(*this);
		trace.Step = InStep;
		return trace;
	}

	/**
	 * Returns the referenced object.
	 */
	UObject* operator*() const;

	/**
	 * Returns the reference object.
	 */
	UObject* Get() const;

	/**
	 * Accesses the referenced object.
	 */
	UObject* operator->() const;

	/**
	 * Creates a new NetworkTrace with the new given object but the same trace as this.
	 * Trys to update the trace step, if no suitable step is found, step will be always valid.
	 */
	FFINNetworkTrace operator()(UObject* Other) const;

	/**
	 * Checks if the traces are the same by just the underlying object
	 */
	bool operator==(const FFINNetworkTrace& Other) const;

	/**
	 * Checks if the trace is valid.
	 * If not, throws a exception.
	 */
	void CheckTrace() const;

	/**
	 * Returnes a reverced version of this trace.
	 * Updates every step on the way accordingly
	 */
	FFINNetworkTrace Reverse() const;

	/**
	 * Executes the step function of it self and cascades the steps of the previous traces.
	 * If no step is found just does the previous traces.
	 */
	bool IsValid() const;

	/**
	 * Checks if the objects of both traces are the same
	 */
	bool IsEqualObj(const FFINNetworkTrace& Other) const;


	/**
	 * Checks if the given trace is larger than self by the underlying objects
	 */
	bool operator<(const FFINNetworkTrace& Other) const;

	/**
	 * returns the underlying weak object ptr without any checks
	 */
	UObject* GetUnderlyingPtr() const;

	/**
	 * returns true if the underlying weak object ptr is valid or not, without checking the trace
	 */
	bool IsValidPtr() const;

	/**
	 * returns the starting object of the trace
	 */
	UObject* GetStartPtr() const;

	/**
	 * Returns the instance of the previous network trace, essentially exposing the whole trace-path.
	 */
	TSharedPtr<const FFINNetworkTrace> GetPrev() const {
		return Prev;
	}

	/**
	 * Returns the currently used step function
	 */
	TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> GetStep() const {
		return Step;
	}
	
	/**
	 * returns if the trace is valid or not
	 */
	operator bool() const;
};

inline FArchive& operator<<(FArchive& Ar, FFINNetworkTrace& trace) {
	trace.Serialize(FStructuredArchiveFromArchive(Ar).GetSlot());
	return Ar;
}

inline void operator<<(FStructuredArchive::FSlot Slot, FFINNetworkTrace& Trace) {
	Trace.Serialize(Slot);
}

FORCEINLINE uint32 GetTypeHash(const FFINNetworkTrace& Trace) {
	return GetTypeHash(Trace.GetUnderlyingPtr());
}

template<>
struct TStructOpsTypeTraits<FFINNetworkTrace> : TStructOpsTypeTraitsBase2<FFINNetworkTrace> {
	enum {
		WithStructuredSerializer = true,
		WithAddStructReferencedObjects = true,
		WithCopy = true,
		WithIdenticalViaEquality = true,
		
    };
};
