#pragma once

#include "FGSaveInterface.h"
#include "Processor.generated.h"

class AFINStateEEPROM;

class UFINKernelSystem;

/**
 * A Processor handles the whole execution of a program and also makes sure that resource overuse causes a crash.
 */
UCLASS()
class FICSITNETWORKS_API UFINKernelProcessor : public UObject, public IFGSaveInterface {
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	UFINKernelSystem* Kernel = nullptr;
	
	UPROPERTY(SaveGame)
	bool bTest = false;
	
public:
	FString DebugInfo;
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface
	
	/**
	 * Sets the kernel this processor uses.
	 *
	 * @param[in]	InKernel	the new kernel the processor will use
	 */
	virtual void SetKernel(UFINKernelSystem* InKernel);

	/**
	 * Allows to access the connected kernel
	 *
	 * @return	returns the connected kernel
	 */
	UFINKernelSystem* GetKernel();

	/**
	 * Does one processor cycle.
	 * Processor needs to execute its speed accordingly.
	 *
	 * Basically redirects the factory tick
	 *
	 * @param[in]	InDeltaTime		the delta seconds since last tick
	 */
	virtual void Tick(float InDeltaTime) {}

	/**
	 * Gets called when the kernel stops or crashes.
	 *
	 * @param[in]	InIsCrash		true if the stop is caused by an crash
	 */
	virtual void Stop(bool InIsCrash) {}

	/**
	 * recalculates the processor memory usage
	 *
	 * @param[in]	InRecalc	set this to true if you want to force the processor to recalculate its memory usage
	 */
	virtual int64 GetMemoryUsage(bool InRecalc = false) { return 0; }

	/**
	 * Resets the execution state of the processor.
	 * f.e. resets the code counter
	 */
	virtual void Reset() {}

	/**
	 * Sets the BIOS code of the processor.
	 * Usage and events depend on implementation (f.e. reset on set)
	 */
	virtual void SetEEPROM(AFINStateEEPROM* InEEPROM) {}
};
