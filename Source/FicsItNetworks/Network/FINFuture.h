#pragma once

#include "CoreMinimal.h"
#include "Reflection/FINFunction.h"
#include "FicsItNetworksModule.h"
#include "FINFuture.generated.h"

USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINFuture {
	GENERATED_BODY()

	virtual ~FFINFuture() = default;

	/**
	 * This function will get called from the FicsIt-Kernel in the main thread if it is added to the
	 * future queue of the kernel.
	 */
	virtual void Execute() {}
	
	/**
	 * Checks if the future has finished and we can get values from it.
	 */
	virtual bool IsDone() const { return false; }

	/**
	 * Returns the output data of the future
	 */
	virtual TArray<FFINAnyNetworkValue> GetOutput() const { return {}; }
};

USTRUCT()
struct FICSITNETWORKS_API FFINFutureReflection : public FFINFuture {
	GENERATED_BODY()

	UPROPERTY()
	bool bDone = false;

	UPROPERTY()
	TArray<FFINAnyNetworkValue> Input;

	UPROPERTY()
	TArray<FFINAnyNetworkValue> Output;

	UPROPERTY()
	FFINExecutionContext Context;

	UPROPERTY()
	UFINFunction* Function;

	FFINFutureReflection() = default;
	FFINFutureReflection(UFINFunction* Function, const FFINExecutionContext& Context, const TArray<FFINAnyNetworkValue>& Input) : Input(Input), Context(Context), Function(Function) {}

	bool Serialize(FArchive& Ar) {
		Ar << bDone;
		Ar << Input;
		Ar << Output;
		Ar << Context;
		Ar << Function;
		return true;
	}

	virtual bool IsDone() const override { return bDone; }

	virtual void Execute() override {
		if (!Function) {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Future unable to get executed due to invalid function pointer!"));
			return;
		}
		Output = Function->Execute(Context, Input);
		bDone = true;
	}

	virtual TArray<FFINAnyNetworkValue> GetOutput() const override {
		return Output;
	}
};

template<>
struct TStructOpsTypeTraits<FFINFutureReflection> : public TStructOpsTypeTraitsBase2<FFINFutureReflection> {
	enum {
		WithSerializer = true,
	};
};

USTRUCT()
struct FICSITNETWORKS_API FFINFunctionFuture : public FFINFuture {
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
};
