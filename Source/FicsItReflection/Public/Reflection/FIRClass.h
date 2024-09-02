#pragma once

#include "CoreMinimal.h"
#include "FIRStruct.h"
#include "FIRSignal.h"
#include "UObject/UObjectIterator.h"
#include "FIRClass.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRClass : public UFIRStruct {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<UFIRSignal*> Signals;

	/**
	 * Returns the parent class of this class
	 */
	UFIRClass* GetParentClass() const {
		UFIRStruct* CurParent = GetParent();
		while (!Cast<UFIRClass>(CurParent) && CurParent) {
			CurParent = CurParent->GetParent();
		}
		return Cast<UFIRClass>(CurParent);
	}

	/**
	 * Returns a list of all available signals
	 * @param[in]	bRecursive	true if signals of all parent classes should be included
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFIRSignal*> GetSignals(bool bRecursive = true) const {
		TArray<UFIRSignal*> Sigs = Signals;
		if (bRecursive && GetParentClass()) Sigs.Append(GetParentClass()->GetSignals(true));
		return Sigs;
	}

	/**
	 * Returns a array filled with all direct child classes of this class.
	 */
	TArray<UFIRClass*> GetChildClasses() const {
		TArray<UFIRClass*> Childs;
		for(TObjectIterator<UFIRClass> It; It; ++It) {
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
	UFIRSignal* FindFIRSignal(const FString& Name);
};
