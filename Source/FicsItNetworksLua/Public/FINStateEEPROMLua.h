#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Processor/FINStateEEPROM_Legacy.h"
#include "FINStateEEPROMLua.generated.h"

UCLASS()
class FICSITNETWORKSLUA_API AFINStateEEPROMLua : public AFINStateEEPROM_Legacy {
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	FString Code;

public:
	UFUNCTION(BlueprintCallable, Category="Computer")
	FString GetCode() const;

	UFUNCTION(BlueprintCallable, Category="Computer")
	void SetCode(const FString& NewCode);
};
