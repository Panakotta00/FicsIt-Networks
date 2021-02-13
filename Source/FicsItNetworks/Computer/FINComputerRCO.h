#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FINComputerCase.h"
#include "FINComputerDriveHolder.h"
#include "FINComputerGPUT1.h"
#include "FicsItKernel/Processor/Lua/FINStateEEPROMLua.h"
#include "FINComputerRCO.generated.h"

UCLASS(Blueprintable)
class FICSITNETWORKS_API UFINComputerRCO : public UFGRemoteCallObject {
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
	
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="Computer|RCO")
	void SetNick(UObject* Component, const FString& Nick);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="Computer|RCO")
	void GPUMouseEvent(AFINComputerGPUT1* GPU, int type, int x, int y, int btn);
	
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="Computer|RCO")
	void GPUKeyEvent(AFINComputerGPUT1* GPU, int type, int64 c, int64 code, int btn);

	UFUNCTION(Server, WithValidation, Reliable)
	void CreateEEPROMState(UFGInventoryComponent* Inv, int SlotIdx);
};
