#pragma once

#include "CoreMinimal.h"

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
			static std::unique_ptr<TraceStep> fallbackTraceStep;
			static std::map<std::pair<UClass*, UClass*>, std::unique_ptr<TraceStep>> traceSteps;

			NetworkTrace* prev = nullptr;
			TraceStep* step = nullptr;
			TWeakObjectPtr<UObject> obj;

		public:
			/**
			 * Adds a new trace step type.
			 * !IMPORTANT! step gets fully occupied, that means you should afterwards never free the pointer to step.
			 */
			static void registerTraceStep(UClass* A, UClass* B, TraceStep* step);

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