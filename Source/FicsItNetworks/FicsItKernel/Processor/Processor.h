#pragma once

#include "CoreMinimal.h"
#include "Json.h"

#include <string>

namespace FicsItKernel {
	enum ProcessorArchitecture {
		LUA
	};

	class KernelSystem;

	/**
	 * A Processor handles the whole execution of a program and also makes sure that resource overusage causes a crash.
	 */
	class Processor {
	protected:
		KernelSystem* kernel;

	public:
		virtual ~Processor() {}

		/**
		* Sets the kernel this processor uses.
		*
		* @param[in]	kernel	the new kernel the processor will use
		*/
		void setKernel(KernelSystem* kernel);

		/**
		* Allows to access the connected kernel
		*
		* @return	returns the connected kernel
		*/
		KernelSystem* getKernel();

		/**
		* Does one processor cycle.
		* Processor needs to execute its speed accordingly.
		*
		* Basically redirects the factory tick
		*
		* @param[in]	delta	the delta seconds since last tick
		*/
		virtual void tick(float delta) = 0;

		/**
		* recalculates the processor memory usage
		*
		* @param[in]	recalc	set this to true if you want to force the processor to recalculate its memory usage
		*/
		virtual int64 getMemoryUsage(bool recalc = false) = 0;

		/**
		* Resets the execution state of the processor.
		* f.e. resets the code counter
		*/
		virtual void reset() = 0;

		/**
		 * De/Serializes the processor state from/to a archive
		 *
		 * @param[in]	Ar	the archive we read/write the state to
		 */
		virtual void Serialize(FArchive& Ar) = 0;
	};
}