#pragma once

#include "CoreMinimal.h"
#include "FIRProperty.h"
#include "FIRSignal.generated.h"

UENUM()
enum EFIRSignalFlags {
	FIR_Signal_None			= 0,
	FIR_Signal_StaticSource	= 0b1,
};

ENUM_CLASS_FLAGS(EFIRSignalFlags)

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRSignal : public UFIRBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	bool bIsVarArgs = false;
	UPROPERTY()
	TArray<UFIRProperty*> Parameters;

	EFIRSignalFlags SignalFlags = FIR_Signal_None;

	/**
	 * Returns the signal flags of this struct
	 */
	virtual EFIRSignalFlags GetSignalFlags() const { return SignalFlags; }
	
	/**
	 * Returns the parameters of the signal
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFIRProperty*> GetParameters() const { return Parameters; }

	/**
	 * Returns if the signal sends a variable amount of arguments
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual bool IsVarArgs() const { return bIsVarArgs; }

	/**
	 * Triggers the Signal
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual void Trigger(UObject* Context, const TArray<FFIRAnyValue>& Data);
};
