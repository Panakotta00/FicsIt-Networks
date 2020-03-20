#include "NetworkTrace.h"

#include "FGPowerConnectionComponent.h"
#include "FGPowerCircuit.h"

#define StepFuncName(A, B) Step ## _ ## A ## _ ## B
#define StepRegName(A, B) StepReg ## _ ## A ## _ ## B
#define StepFuncSig(A, B) bool StepFuncName(A, B)(UObject* oA, UObject* oB)
#define Step(CA, CB) \
	StepFuncSig(CA, CB); \
	TraceStepRegisterer StepRegName(CA, CB)(CA::StaticClass(), CB::StaticClass(), &StepFuncName(CA, CB)); \
	StepFuncSig(CA, CB) { \
		CA* A = Cast<CA>(oA); \
		CB* B = Cast<CB>(oB);

namespace FicsItKernel {
	namespace Network {
		class TraceStepRegisterer {
		public:
			TraceStepRegisterer(UClass* A, UClass* B, bool(*func)(UObject*, UObject*)) {
				NetworkTrace::registerTraceStep(A, B, new TraceStep(func));
			}
		};

		std::unique_ptr<TraceStep> NetworkTrace::fallbackTraceStep;
		std::map<std::pair<UClass*, UClass*>, std::unique_ptr<TraceStep>> NetworkTrace::traceSteps;

		void NetworkTrace::registerTraceStep(UClass* A, UClass* B, TraceStep* step) {
			traceSteps[{A, B}] = std::unique_ptr<TraceStep>(step);
		}
		
		TraceStep* NetworkTrace::findTraceStep(UClass* A, UClass* B) {
			auto step = traceSteps.find({A, B});
			if (step == traceSteps.end()) {
				// no valid trace step found
				return fallbackTraceStep.get();
			}
			return step->second.get();
		}

		NetworkTrace::NetworkTrace() : NetworkTrace(nullptr) {}

		NetworkTrace::NetworkTrace(UObject* obj) : obj(obj) {}

		NetworkTrace::~NetworkTrace() {
			if (prev) delete prev;
		}
		
		NetworkTrace NetworkTrace::operator/(UObject* other) const {
			NetworkTrace trace(other);

			auto A = obj.Get();
			if (!IsValid(A) || !IsValid(other)) return NetworkTrace(nullptr); // if A is not valid, the network trace will always be now invalid
			trace.prev = new NetworkTrace(*this);
			trace.step = findTraceStep(A->GetClass(), other->GetClass());
			return trace;
		}

		NetworkTrace NetworkTrace::operator/(std::pair<UObject*, TraceStep*> other) {
			NetworkTrace trace(other.first);
			trace.prev = new NetworkTrace(*this);
			trace.step = other.second;
			return trace;
		}

		UObject* NetworkTrace::operator*() const {
			UObject* B = obj.Get();
			if (prev) {
				UObject* A = **prev;
				if (!IsValid(A) || !IsValid(B)) return nullptr;
				return (*step)(A, B) ? B : nullptr;
			} else if (IsValid(B)) {
				return B;
			} else {
				return nullptr;
			}
		}

		UObject* NetworkTrace::operator->() const {
			return **this;
		}

		NetworkTrace NetworkTrace::operator()(UObject* other) const {
			if (!IsValid(other)) return NetworkTrace(other);

			NetworkTrace trace(*this);
			trace.obj = other;
			
			if (trace.prev) {
				auto A = trace.prev->obj.Get();
				if (!IsValid(A)) return NetworkTrace(nullptr); // if the previous network trace object is invalid, the trace will be always invalid
				trace.step = findTraceStep(trace.prev->obj->GetClass(), other->GetClass());
			}

			return trace;
		}

		bool NetworkTrace::operator==(const NetworkTrace& other) const {
			return obj == other.obj;
		}

		void NetworkTrace::checkTrace() const {
			if (!**this) throw std::exception("Object is not reachable");
		}

		NetworkTrace NetworkTrace::reverse() const {
			if (!obj.IsValid()) return NetworkTrace(nullptr);
			NetworkTrace trace(obj.Get());
			NetworkTrace* prev = this->prev;
			while (prev) {
				trace = trace / prev->obj.Get();
				prev = prev->prev;
			}
			return trace;
		}

		bool NetworkTrace::isEqualObj(const NetworkTrace& other) const {
			return obj == other.obj;
		}

		bool NetworkTrace::operator<(const NetworkTrace& other) const {
			struct TWOP {
				int32		ObjectIndex;
				int32		ObjectSerialNumber;
			};

			TWOP* d1 = (TWOP*)&obj;
			TWOP* d2 = (TWOP*)&other.obj;
			if (d1->ObjectIndex < d2->ObjectIndex) return true;
			else return d1->ObjectSerialNumber < d2->ObjectSerialNumber;
		}

		TWeakObjectPtr<UObject> NetworkTrace::getUnderlyingPtr() const {
			return obj;
		}

		/* ############### */
		/* # Trace Steps # */
		/* ############### */
		
		Step(UFGPowerConnectionComponent, UFGPowerCircuit)
			return A->GetCircuitID() == B->GetCircuitID();
		}
	}
}