#include "FIRTrace.h"

TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> FFIRTrace::fallbackTraceStep;
TArray<TPair<TPair<UClass*, UClass*>, TPair<FString, FFINTraceStep*>>(*)()> FFIRTrace::toRegister;
TMap<FString, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>> FFIRTrace::traceStepRegistry;
TMap<TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>, FString> FFIRTrace::inverseTraceStepRegistry;
TMap<UClass*, TPair<TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>, TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>>> FFIRTrace::traceStepMap;
TMap<UClass*, TPair<TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>, TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>>> FFIRTrace::interfaceTraceStepMap;

TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> findTraceStep2(TPair<TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>, TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>>& stepList, UClass* B) {
	UClass* Bi = B;
	while (Bi && Bi != UObject::StaticClass()) {
		auto stepB = stepList.Key.Find(Bi);
		if (stepB) {
			return *stepB;
		}
		Bi = Bi->GetSuperClass();
	}

	for (FImplementedInterface& interface : B->Interfaces) {
		auto stepB = stepList.Value.Find(interface.Class);
		if (stepB) {
			return *stepB;
		}
	}

	return nullptr;
}

TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> FFIRTrace::findTraceStep(UClass* A, UClass* B) {
	if (!A || !B) return fallbackTraceStep;
	UClass* Ai = A;
	while (Ai && Ai != UObject::StaticClass()) {
		auto stepA = traceStepMap.Find(Ai);
		if (stepA) {
			TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> step = findTraceStep2(*stepA, B);
			if (step.IsValid()) return step;
		}
		Ai = Ai->GetSuperClass();
	}
	
	for (FImplementedInterface& interface : A->Interfaces) {
		auto stepA = interfaceTraceStepMap.Find(interface.Class);
		if (stepA) {
			TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> step = findTraceStep2(*stepA, B);
			if (step.IsValid()) return step;
		}
	}

	return fallbackTraceStep;
}

FFIRTrace::FFIRTrace(const FFIRTrace& trace) {
	// TODO: traceRegisterSteps();
	
	Prev = MakeShareable((trace.Prev) ? new FFIRTrace(*trace.Prev) : nullptr);
	Step = trace.Step;
	Obj = trace.Obj;
}

FFIRTrace& FFIRTrace::operator=(const FFIRTrace& trace) {
	Prev = MakeShareable((trace.Prev) ? new FFIRTrace(*trace.Prev) : nullptr);
	Step = trace.Step;
	Obj = trace.Obj;

	return *this;
}

FFIRTrace::FFIRTrace() : FFIRTrace(nullptr) {
	// TODO: traceRegisterSteps();
}

FFIRTrace::FFIRTrace(UObject* Obj) : Obj(Obj) {
	// TODO: traceRegisterSteps();
}

FFIRTrace::~FFIRTrace() {}

bool FFIRTrace::Serialize(FStructuredArchive::FSlot Slot) {
	if (Slot.GetUnderlyingArchive().IsSaveGame()) {
		FStructuredArchive::FRecord Record = Slot.EnterRecord();
		if (!::IsValid(Obj)) Obj = nullptr;
		Record.EnterField(SA_FIELD_NAME(TEXT("Ptr"))) << Obj;

		TOptional<FStructuredArchive::FSlot> PrevSlot = Record.TryEnterField(SA_FIELD_NAME(TEXT("Next")), Prev.IsValid());
		if (PrevSlot.IsSet()) {
			if (!Prev.IsValid()) Prev = MakeShared<FFIRTrace>();
			Prev->Serialize(PrevSlot.GetValue());
		} else {
			Prev.Reset();
		}

		TOptional<FStructuredArchive::FSlot> StepSlot = Record.TryEnterField(SA_FIELD_NAME(TEXT("Step")), Step.IsValid());
		if (StepSlot.IsSet()) {
			FString StepName;
			if (Step.IsValid()) StepName = inverseTraceStepRegistry[Step];
			StepSlot.GetValue() << StepName;
			TSharedPtr<FFINTraceStep>* StepPtr = traceStepRegistry.Find(StepName);
			if (StepPtr) {
				Step = *StepPtr;
			}
		} else {
			Step.Reset();
		}

		return true;
	}
	
	return false;
}

void FFIRTrace::AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const {
	if (Obj) {
		ReferenceCollector.AddReferencedObject(const_cast<UObject*&>(Obj));
		if (Obj) Obj->CallAddReferencedObjects(ReferenceCollector);
	}
	if (Prev.IsValid()) {
		Prev->AddStructReferencedObjects(ReferenceCollector);
	}
}

FFIRTrace FFIRTrace::operator/(UObject* other) const {
	FFIRTrace trace(other);

	UObject* A = Obj;
	if (!::IsValid(A) || !other) return FFIRTrace(nullptr); // if A is not valid, the network trace will always be not invalid
	trace.Prev = MakeShared<FFIRTrace>(*this);
	trace.Step = findTraceStep(A->GetClass(), other->GetClass());
	return trace;
}

UObject* FFIRTrace::operator*() const {
	return Obj;
}

UObject* FFIRTrace::Get() const {
	return Obj;
}

UObject* FFIRTrace::operator->() const {
	return Obj;
}

FFIRTrace FFIRTrace::operator()(UObject* other) const {
	if (!other) return FFIRTrace(nullptr);

	FFIRTrace trace(*this);
	trace.Obj = other;
	
	if (trace.Prev) {
		auto A = trace.Prev->Obj;
		if (!::IsValid(A)) return FFIRTrace(nullptr); // if the previous network trace object is invalid, the trace will be always invalid
		trace.Step = findTraceStep(trace.Prev->Obj->GetClass(), other->GetClass());
	}

	return trace;
}

bool FFIRTrace::operator==(const FFIRTrace& other) const {
	return Obj == other.Obj;
}

FFIRTrace FFIRTrace::Reverse() const {
	if (!::IsValid(Obj)) return FFIRTrace(nullptr);
	FFIRTrace trace(Obj);
	TSharedPtr<FFIRTrace> prev;
	if (this->Prev) prev = MakeShared<FFIRTrace>(*this->Prev);
	while (prev) {
		trace = trace / prev->Obj;
		prev = prev->Prev;
	}
	return trace;
}

bool FFIRTrace::IsValid() const {
	UObject* B = Obj;
	if (!::IsValid(B)) return false;
	if (Prev && Step && *Step) {
		UObject* A = Prev->Obj;
		if (!A || !(*Step)(A, B)) return false;
	}
	if (Prev) {
		return Prev->IsValid();
	}
	return true;
}

bool FFIRTrace::IsEqualObj(const FFIRTrace& other) const {
	return Obj == other.Obj;
}

bool FFIRTrace::operator<(const FFIRTrace& other) const {
	struct TWOP {
		int32		ObjectIndex;
		int32		ObjectSerialNumber;
	};
	TWOP* d1 = (TWOP*)&Obj;
	TWOP* d2 = (TWOP*)&other.Obj;
	if (d1->ObjectIndex < d2->ObjectIndex) return true;
	else return d1->ObjectSerialNumber < d2->ObjectSerialNumber;
}

UObject* FFIRTrace::GetUnderlyingPtr() const {
	return Obj;
}

bool FFIRTrace::IsValidPtr() const {
	return ::IsValid(GetUnderlyingPtr());
}

UObject* FFIRTrace::GetStartPtr() const {
	const FFIRTrace* Trace = this;
	while (Trace->Prev) Trace = Trace->Prev.Get();
	if (Trace) return Trace->Obj;
	return nullptr;
}

FFIRTrace::operator bool() const {
	return IsValid();
}
