#pragma once

#include "CoreMinimal.h"
#include "FINValueReader.h"
#include "Reflection/FINFunction.h"


#include "FINFuture.generated.h"

USTRUCT(BlueprintType)
struct FFINFuture {
	GENERATED_BODY()

	virtual ~FFINFuture() = default;

	/**
	 * This function will get called from the FicsIt-Kernel in the main thread if it is added to the
	 * future queue of the kernel.
	 */
	virtual void Execute() {}

	/**
	 * This function write the resulting values of the future to the given reader.
	 */
	virtual int operator>>(FFINValueReader& Reader) const { return 0; }

	/**
	 * Checks if the future has finished and we can get values from it.
	 */
	virtual bool IsDone() const { return false; }
};

USTRUCT()
struct FFINFutureReflection : public FFINFuture {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	bool bDone = false;

	UPROPERTY(SaveGame)
	TArray<FFINAnyNetworkValue> Input;

	UPROPERTY(SaveGame)
	TArray<FFINAnyNetworkValue> Output;

	UPROPERTY(SaveGame)
	FFINNetworkTrace Context;

	UPROPERTY(SaveGame)
	UFINFunction* Function;

	FFINFutureReflection() = default;
	FFINFutureReflection(UFINFunction* Function, const FFINNetworkTrace& Context, const TArray<FFINAnyNetworkValue>& Input) : Input(Input), Context(Context), Function(Function) {}

	virtual bool IsDone() const override { return bDone; }

	virtual void Execute() override {
		if (!Function) {
			SML::Logging::error("Future unable to get executed due to invalid function pointer!");
			return;
		}
		Output = Function->Execute(Context, Input);
		bDone = true;
	}
};

USTRUCT()
struct FFINFutureSimpleDone : public FFINFuture {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	bool bDone = false;

	virtual bool IsDone() const override { return bDone; }
};

USTRUCT()
struct FFINFunctionFuture : public FFINFuture {
	GENERATED_BODY()

	TFunction<void()> Func;

	bool bDone = false;

	FFINFunctionFuture() = default;
	FFINFunctionFuture(TFunction<void()> Func) : Func(Func) {}

	virtual void Execute() override {
		bDone = true;
		Func();
	}

	virtual bool IsDone() const override { return bDone; }

	virtual int operator>>(FFINValueReader& Reader) const override { return 0; }
};