#pragma once

#include "FINStruct.h"
#include "FINRefSignal.h"
#include "FINClass.generated.h"

UCLASS(BlueprintType)
class UFINClass : public UFINStruct {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<UFINRefSignal*> Signals;

	/**
	 * Returns the parent class of this class
	 */
	UFINClass* GetParentClass() const {
		UFINStruct* Parent = GetParent();
		while (!Cast<UFINClass>(Parent) && Parent) {
			Parent = Parent->GetParent();
		}
		return Cast<UFINClass>(Parent);
	}

	/**
	 * Returns a list of all available signals
	 * @param[in]	bRecursive	true if signals of all parent classes should be included
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINRefSignal*> GetSignals(bool bRecursive = true) const {
		TArray<UFINRefSignal*> Sigs = Signals;
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
};
