#pragma once

#include "FINStruct.h"
#include "FINSignal.h"
#include "FINClass.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINClass : public UFINStruct {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<UFINSignal*> Signals;

	/**
	 * Returns the parent class of this class
	 */
	UFINClass* GetParentClass() const {
		UFINStruct* CurParent = GetParent();
		while (!Cast<UFINClass>(CurParent) && CurParent) {
			CurParent = CurParent->GetParent();
		}
		return Cast<UFINClass>(CurParent);
	}

	/**
	 * Returns a list of all available signals
	 * @param[in]	bRecursive	true if signals of all parent classes should be included
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINSignal*> GetSignals(bool bRecursive = true) const {
		TArray<UFINSignal*> Sigs = Signals;
		if (bRecursive && GetParentClass()) Sigs.Append(GetParentClass()->GetSignals(true));
		return Sigs;
	}

	/**
	 * Returns a array filled with all direct child classes of this class.
	 */
	TArray<UFINClass*> GetChildClasses() const {
		TArray<UFINClass*> Childs;
		for(TObjectIterator<UFINClass> It; It; ++It) {
			if(It->GetParent() == this) {
				Childs.Add(*It);
			}
		}
		return Childs;
	}

	/**
	 * Trys to find a signal with the given name in this class
	 * @param[in]	Name	the internal name of the signal you try to find
	 * @return	the found signal or nullptr
	 */
	UFINSignal* FindFINSignal(const FString& Name);
};
