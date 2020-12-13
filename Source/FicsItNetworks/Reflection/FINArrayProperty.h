#pragma once

#include "FINFuncProperty.h"
#include "FINArrayProperty.generated.h"

UCLASS(BlueprintType)
class UFINArrayProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UArrayProperty* Property = nullptr;
	UPROPERTY()
	UFINProperty* InnerType = nullptr;
	
	// Begin UFINProperty
	virtual FINAny GetValue(const FFINExecutionContext& Ctx) const override;
	virtual void SetValue(const FFINExecutionContext& Ctx, const FINAny& Value) const override;
	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_ARRAY; }
	// End UFINProperty

	/**
	 * Returns the array as FINArray
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	TArray<FFINAnyNetworkValue> GetArray(UObject* Ctx) const { return GetArray((void*)Ctx); }
	virtual FINArray GetArray(void* Ctx) const;

	/**
	 * Sets the array to the FINArray
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void SetArray(UObject* Ctx, const TArray<FFINAnyNetworkValue>& Array) const { SetArray((void*)Ctx, Array); }
	virtual void SetArray(void* Ctx, const FINArray& Array) const;

	/**
	 * Set value in the array in the given container
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void AddValueToArray(UObject* Ctx, const FFINAnyNetworkValue& Value) const { AddValueToArray((void*)Ctx, Value); }
	virtual void AddValueToArray(void* Ctx, const FINAny& Value) const;

	/**
	 * Returns the value at the given index in the given container array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual FFINAnyNetworkValue GetValueInArray(UObject* Ctx, int Index) const { return GetValueInArray((void*)Ctx, Index); }
	FINAny GetValueInArray(void* Ctx, int Index) const;

	/**
	 * Sets the value at the given index in the given container array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void SetValueInArray(UObject* Ctx, int Index, const FFINAnyNetworkValue& Value) const { SetValueInArray((void*)Ctx, Index, Value); }
	virtual void SetValueInArray(void* Ctx, int Index, const FINAny& Value) const;

	/**
	 * Removes the value at the given index in the given container array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void RemoveValueInArray(UObject* Ctx, int Index) const { RemoveValueInArray((void*)Ctx, Index); }
	virtual void RemoveValueInArray(void* Ctx, int Index) const;

	/**
	 * Empties the whole array at the given index in the given container array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void EmptyArray(UObject* Ctx) const { EmptyArray((void*)Ctx); }
	virtual void EmptyArray(void* Ctx) const;

	/**
	 * Returns the number of entries in the array in the given container
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	int GetNumOfArray(UObject* Ctx) const { return GetNumOfArray((void*)Ctx); }
	virtual int GetNumOfArray(void* Ctx) const;

	/**
	 * Returns the inner type property of the array
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual UFINProperty* GetInnerType() const { return InnerType; }

	
};
