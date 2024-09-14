#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Processor/FINStateEEPROM.h"
#include "FINStateEEPROMText.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINStateEEPROMText : public AFINStateEEPROM {
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
