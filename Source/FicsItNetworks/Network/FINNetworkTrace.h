#pragma once

#include "CoreMinimal.h"
#include "util/Logging.h"

#include "FINNetworkTrace.generated.h"

/**
* Implements a trace validation step.
* Checks if object B is reachable from object A.
* !IMPORTANT! A and B should be valid pointers and no nullptr.
*/
typedef TFunction<bool(UObject*, UObject*)> FFINTraceStep;

/**
* Used for the extension of a trace to also pass a custom trace step
*/
typedef TPair<UObject*, TSharedPtr<FFINTraceStep>> FFINObjTraceStepPtr;
inline FFINObjTraceStepPtr ObjTraceStep(UObject* obj, FFINTraceStep step) {
	return FFINObjTraceStepPtr(obj, MakeShared<FFINTraceStep>(step));
}

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
	TSharedPtr<FFINTraceStep> Step = nullptr;
	TWeakObjectPtr<UObject> Obj = nullptr;

public:
	static TSharedPtr<FFINTraceStep> fallbackTraceStep;
	static TArray<TPair<TPair<UClass*, UClass*>, TPair<FString, FFINTraceStep*>>(*)()> toRegister;
	static TMap<FString, TSharedPtr<FFINTraceStep>> traceStepRegistry;
	static TMap<TSharedPtr<FFINTraceStep>, FString> inverseTraceStepRegistry;
	static TMap<UClass*, TPair<TMap<UClass*, TSharedPtr<FFINTraceStep>>, TMap<UClass*, TSharedPtr<FFINTraceStep>>>> traceStepMap;
	static TMap<UClass*, TPair<TMap<UClass*, TSharedPtr<FFINTraceStep>>, TMap<UClass*, TSharedPtr<FFINTraceStep>>>> interfaceTraceStepMap;

	/**
	* Trys to find the most suitable trace step of for both given classes
	*/
	static TSharedPtr<FFINTraceStep> findTraceStep(UClass* A, UClass* B);
	
	FFINNetworkTrace(const FFINNetworkTrace& trace);
	FFINNetworkTrace& operator=(const FFINNetworkTrace& trace);

	explicit FFINNetworkTrace();
	explicit FFINNetworkTrace(UObject* obj);
	~FFINNetworkTrace();

	bool Serialize(FArchive& Ar);

	/**
	 * Creates a copy of this network trace and adds potentially a new optimal trace step
	 * from the current referenced object to the new given one.
	 * Will set the referenced object of the copy to the given object.
	 * If no optimal trace step is found, works like operator()
	 * @return the copied and expanded network trace
	 */
	FFINNetworkTrace operator/(UObject* other) const;

	/**
	 * Creates a copy of this network trace appends the given object and uses the given trace step for validation.
	 */
	FFINNetworkTrace operator/(FFINObjTraceStepPtr other);

	/**
	 * Returns the referenced object.
	 * nullptr if trace is invalid
	 */
	UObject* operator*() const;

	/**
	 * Accesses the referenced object.
	 * nullptr if trace is invalid
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
	TWeakObjectPtr<UObject> GetUnderlyingPtr() const;
};

inline FArchive& operator<<(FArchive& Ar, FFINNetworkTrace& trace) {
	trace.Serialize(Ar);
	return Ar;
}

inline bool FFINNetworkTrace::Serialize(FArchive& Ar) {
	if (Ar.IsSaveGame()) {
		bool valid = GetUnderlyingPtr().IsValid();
		Ar << valid;
		if (valid) {
			// obj ptr
			UObject* ptr = GetUnderlyingPtr().Get();
			Ar << ptr;
			Obj = ptr;
	
			// prev trace
			bool hasPrev = Prev.IsValid();
			Ar << hasPrev;
			if (hasPrev) {
				FFINNetworkTrace prev;
				if (Ar.IsSaving()) prev = *Prev;
				Ar << prev;
				if (Ar.IsLoading()) {
					Prev = MakeShared<FFINNetworkTrace>(prev);
				}
			}

			// step
			bool hasStep = Step.Get();
			Ar << hasStep;
			if (hasStep) {
				FString save;
				if (Ar.IsSaving()) save = inverseTraceStepRegistry[Step];
				Ar << save;
				if (Ar.IsLoading()) Step = traceStepRegistry[TCHAR_TO_UTF8(*save)];
			}
		}
	}
	
	return true;
}

FORCEINLINE uint32 GetTypeHash(const FFINNetworkTrace& Trace) {
	return GetTypeHash(Trace.GetUnderlyingPtr());
}