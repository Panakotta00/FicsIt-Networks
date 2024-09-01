#pragma once

#include "CoreMinimal.h"
#include "FIRTrace.generated.h"

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
struct FICSITREFLECTION_API FFIRTrace {
	GENERATED_BODY()

	friend uint32 GetTypeHash(const FFIRTrace&);

private:
	TSharedPtr<FFIRTrace> Prev = nullptr;
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
	
	FFIRTrace(const FFIRTrace& trace);
	FFIRTrace& operator=(const FFIRTrace& trace);

	explicit FFIRTrace();
	explicit FFIRTrace(UObject* obj);
	~FFIRTrace();

	bool Serialize(FStructuredArchive::FSlot Slot);
	void AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const;

	/**
	 * Creates a copy of this network trace and adds potentially a new optimal trace step
	 * from the current referenced object to the new given one.
	 * Will set the referenced object of the copy to the given object.
	 * If no optimal trace step is found, works like operator()
	 * @return the copied and expanded network trace
	 */
	FFIRTrace operator/(UObject* other) const;

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
	FFIRTrace operator()(UObject* Other) const;

	/**
	 * Checks if the traces are the same by just the underlying object
	 */
	bool operator==(const FFIRTrace& Other) const;

	/**
	 * Checks if the trace is valid.
	 * If not, throws a exception.
	 */
	void CheckTrace() const;

	/**
	 * Returnes a reverced version of this trace.
	 * Updates every step on the way accordingly
	 */
	FFIRTrace Reverse() const;

	/**
	 * Executes the step function of it self and cascades the steps of the previous traces.
	 * If no step is found just does the previous traces.
	 */
	bool IsValid() const;

	/**
	 * Checks if the objects of both traces are the same
	 */
	bool IsEqualObj(const FFIRTrace& Other) const;


	/**
	 * Checks if the given trace is larger than self by the underlying objects
	 */
	bool operator<(const FFIRTrace& Other) const;

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
	 * returns if the trace is valid or not
	 */
	operator bool() const;
};

inline FArchive& operator<<(FArchive& Ar, FFIRTrace& trace) {
	trace.Serialize(FStructuredArchiveFromArchive(Ar).GetSlot());
	return Ar;
}

inline void operator<<(FStructuredArchive::FSlot Slot, FFIRTrace& Trace) {
	Trace.Serialize(Slot);
}

FORCEINLINE uint32 GetTypeHash(const FFIRTrace& Trace) {
	return GetTypeHash(Trace.GetUnderlyingPtr());
}

template<>
struct TStructOpsTypeTraits<FFIRTrace> : TStructOpsTypeTraitsBase2<FFIRTrace> {
	enum {
		WithStructuredSerializer = true,
		WithAddStructReferencedObjects = true,
		WithCopy = true,
		WithIdenticalViaEquality = true,
		
    };
};
