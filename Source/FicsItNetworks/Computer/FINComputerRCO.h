#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FINComputerCase.h"
#include "FINComputerDriveHolder.h"
#include "FicsItKernel/Processor/Lua/FINStateEEPROMLua.h"
#include "FINComputerRCO.generated.h"

UCLASS(Blueprintable)
class UFINComputerRCO : public UFGRemoteCallObject {
	GENERATED_BODY()
	
public:
	UPROPERTY(Replicated)
	bool bDummy = false;

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="Computer|RCO")
    void SetLuaEEPROMCode(AFINStateEEPROMLua* LuaEEPROMState, const FString& NewCode);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="Computer|RCO")
	void SetCaseLastTab(AFINComputerCase* Case, int LastTab);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="Computer|RCO")
	void SetDriveHolderLocked(AFINComputerDriveHolder* Holder, bool bLocked);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="Computer|RCO")
	void ToggleCase(AFINComputerCase* Case);
};
