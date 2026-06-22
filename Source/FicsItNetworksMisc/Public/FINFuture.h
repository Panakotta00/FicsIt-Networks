#pragma once

#include "CoreMinimal.h"
#include "FicsItNetworksMisc.h"
#include "FIRAnyValue.h"
#include "FIRException.h"
#include "Reflection/FIRExecutionContext.h"
#include "Reflection/FIRFunction.h"
#include "Reflection/FIRProperty.h"
#include "FINFuture.generated.h"

class UFIRFunction;

USTRUCT(BlueprintType)
struct FICSITNETWORKSMISC_API FFINFuture {
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
	virtual TArray<FFIRAnyValue> GetOutput() const { return {}; }
};

USTRUCT()
struct FICSITNETWORKSMISC_API FFINFutureReflection : public FFINFuture {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	bool bDone = false;

	UPROPERTY(SaveGame)
	TArray<FFIRAnyValue> Input;

	UPROPERTY(SaveGame)
	TArray<FFIRAnyValue> Output;

	UPROPERTY(SaveGame)
	FFIRExecutionContext Context;

	UPROPERTY(SaveGame)
	UFIRFunction* Function = nullptr;

	UPROPERTY(SaveGame)
	UFIRProperty* Property = nullptr;

	// BUG #16: If Function->Execute / a property throws an FFIRException on the game thread, it must
	// NOT fly through uncaught (it would become [Fatal] since there is no C++ catch across the Lua C
	// frames -> computer kernel halted). Instead capture it here and hand it back via HasError() to the
	// await continuation, which turns it into a catchable Lua error. Transient (no SaveGame): a failed
	// future is immediately done and is not persisted mid-flight.
	bool bError = false;
	FString ErrorMessage;

	static FCriticalSection Mutex;

	// TODO: Maybe do a LogScope snapshot?
	FFINFutureReflection() = default;
	FFINFutureReflection(UFIRFunction* Function, const FFIRExecutionContext& Context, const TArray<FFIRAnyValue>& Input) : Input(Input), Context(Context), Function(Function) {}
	FFINFutureReflection(UFIRProperty* Property, const FFIRExecutionContext& Context, const FFIRAnyValue& Input) : Input({Input}), Context(Context), Property(Property) {}
	FFINFutureReflection(UFIRProperty* Property, const FFIRExecutionContext& Context) : Context(Context), Property(Property) {}

	virtual bool IsDone() const override {
		FScopeLock Lock(const_cast<FCriticalSection*>(&Mutex));
		return bDone;
	}

	virtual void Execute() override;

	virtual TArray<FFIRAnyValue> GetOutput() const override {
		FScopeLock Lock(const_cast<FCriticalSection*>(&Mutex));
		return Output;
	}

	// BUG #16: true if the game-thread execution caught an exception; OutMessage = the message.
	bool HasError(FString& OutMessage) const {
		FScopeLock Lock(const_cast<FCriticalSection*>(&Mutex));
		if (bError) { OutMessage = ErrorMessage; return true; }
		return false;
	}
};

USTRUCT()
struct FICSITNETWORKSMISC_API FFINFunctionFuture : public FFINFuture {
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
