#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace FicsItKernel {
	namespace Network {
		/**
		 * Implements a trace validation step.
		 * Checks if object B is reachable from object A.
		 * !IMPORTANT! A and B should be valid pointers and no nullptr.
		 */
		typedef std::function<bool(UObject*, UObject*)> TraceStep;

		/**
		 * This class allows for tracing object access over the component network.
		 */
		class NetworkTrace {
		private:
			NetworkTrace* prev = nullptr;
			TraceStep* step = nullptr;
			TWeakObjectPtr<UObject> obj;

		public:
			static std::unique_ptr<TraceStep> fallbackTraceStep;
			static std::vector<std::pair<std::pair<UClass*, UClass*>, TraceStep*>(*)()> toRegister;
			static std::map<UClass*, std::pair<std::map<UClass*, std::unique_ptr<TraceStep>>, std::map<UClass*, std::unique_ptr<TraceStep>>>> traceSteps;
			static std::map<UClass*, std::pair<std::map<UClass*, std::unique_ptr<TraceStep>>, std::map<UClass*, std::unique_ptr<TraceStep>>>> interfaceTraceSteps;

			/**
			 * Trys to find the most suitable trace step of for both given classes
			 */
			static TraceStep* findTraceStep(UClass* A, UClass* B);

			NetworkTrace(const NetworkTrace& trace);
			NetworkTrace(NetworkTrace&& trace);
			NetworkTrace& operator=(const NetworkTrace& trace);
			NetworkTrace& operator=(NetworkTrace&& trace);

			explicit NetworkTrace();
			explicit NetworkTrace(UObject* obj);
			~NetworkTrace();

			/**
			 * Creates a copy of this network trace and adds potentially a new optimal trace step
			 * from the current referenced object to the new given one.
			 * Will set the referenced object of the copy to the given object.
			 * If no optimal trace step is found, works like operator()
			 * @return the copied and expanded network trace
			 */
			NetworkTrace operator/(UObject* other) const;

			/**
			 * Creates a copy of this network trace appends the given object and uses the given trace step for validation.
			 */
			NetworkTrace operator/(std::pair<UObject*, TraceStep*> other);

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
			NetworkTrace operator()(UObject* other) const;

			/**
			 * Checks if the traces are the same by just the underlying object
			 */
			bool operator==(const NetworkTrace& other) const;

			/**
			 * Checks if the trace is valid.
			 * If not, throws a exception.
			 */
			void checkTrace() const;

			/**
			 * Returnes a reverced version of this trace.
			 * Updates every step on the way accordingly
			 */
			NetworkTrace reverse() const;

			/**
			 * Executes the step function of it self and cascades the steps of the previous traces.
			 * If no step is found just does the previous traces.
			 */
			bool isValid() const;

			/**
			 * Checks if the objects of both traces are the same
			 */
			bool isEqualObj(const NetworkTrace& other) const;


			/**
			 * Checks if the given trace is larger than self by the underlying objects
			 */
			bool operator<(const NetworkTrace& other) const;

			/**
			 * returns the underlying weak object ptr without any checks
			 */
			TWeakObjectPtr<UObject> getUnderlyingPtr() const;
		};
	}
}