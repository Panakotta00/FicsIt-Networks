#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Processor/FINStateEEPROM.h"
#include "FINStateEEPROMLua.generated.h"

UCLASS()
class FICSITNETWORKSLUA_API AFINStateEEPROMLua : public AFINStateEEPROM {
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	FString Code;

public:
	UFUNCTION(BlueprintCallable, Category="Computer")
	FString GetCode() const;

	UFUNCTION(BlueprintCallable, Category="Computer")
	void SetCode(const FString& NewCode);

	virtual bool CopyDataTo(AFINStateEEPROM* InFrom) override;
};
