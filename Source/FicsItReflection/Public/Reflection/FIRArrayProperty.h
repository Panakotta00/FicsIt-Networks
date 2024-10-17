#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRArrayProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRArrayProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FArrayProperty* Property = nullptr;
	UPROPERTY()
	UFIRProperty* InnerType = nullptr;
	
	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override;
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override;
	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_ARRAY; }
	// End UFINProperty

	/**
	 * Returns the array as FINArray
	 * Ctx needs to be pointer to the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	TArray<FFIRAnyValue> GetArray(UObject* Ctx) const { return GetArray((void*)Ctx); }
	virtual FIRArray GetArray(void* Ctx) const;

	/**
	 * Sets the array to the FINArray
	 * Ctx needs to be pointer to the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void SetArray(UObject* Ctx, const TArray<FFIRAnyValue>& Array) const { SetArray((void*)Ctx, Array); }
	virtual void SetArray(void* Ctx, const FIRArray& Array) const;

	/**
	 * Set value in the array in the given container
	 * ctx needs to be pointer to the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void AddValueToArray(UObject* Ctx, const FFIRAnyValue& Value) const { AddValueToArray((void*)Ctx, Value); }
	virtual void AddValueToArray(void* Ctx, const FIRAny& Value) const;

	/**
	 * Returns the value at the given index in the given container array
	 * ctx need to be pointer to the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual FFIRAnyValue GetValueInArray(UObject* Ctx, int Index) const { return GetValueInArray((void*)Ctx, Index); }
	FIRAny GetValueInArray(void* Ctx, int Index) const;

	/**
	 * Sets the value at the given index in the given container array
	 * ctx need to be pointer to the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void SetValueInArray(UObject* Ctx, int Index, const FFIRAnyValue& Value) const { SetValueInArray((void*)Ctx, Index, Value); }
	virtual void SetValueInArray(void* Ctx, int Index, const FIRAny& Value) const;

	/**
	 * Removes the value at the given index in the given container array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void RemoveValueInArray(UObject* Ctx, int Index) const { RemoveValueInArray((void*)Ctx, Index); }
	virtual void RemoveValueInArray(void* Ctx, int Index) const;

	/**
	 * Empties the whole array at the given index in the given container array
	 * ctx need to be pointer to the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void EmptyArray(UObject* Ctx) const { EmptyArray((void*)Ctx); }
	virtual void EmptyArray(void* Ctx) const;

	/**
	 * Returns the number of entries in the array in the given container
	 * ctx need to be pointer to the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	int GetNumOfArray(UObject* Ctx) const { return GetNumOfArray((void*)Ctx); }
	virtual int GetNumOfArray(void* Ctx) const;

	/**
	 * Returns the inner type property of the array
	 * ctx need to be pointer to the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual UFIRProperty* GetInnerType() const { return InnerType; }

	
};
