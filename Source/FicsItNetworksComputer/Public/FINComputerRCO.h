#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FINComputerRCO.generated.h"

class AFINComputerGPUT1;
class AFINComputerGPUT2;

UCLASS(Blueprintable)
class FICSITNETWORKSCOMPUTER_API UFINComputerRCO : public UFGRemoteCallObject {
	GENERATED_BODY()
	
public:
	UPROPERTY(Replicated)
	bool bDummy = false;

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void SetCaseLastTab(AFINComputerCase* Case, int LastTab);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void SetDriveHolderLocked(AFINComputerDriveHolder* Holder, bool bLocked);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void ToggleCase(AFINComputerCase* Case);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void SetNick(UObject* Component, const FString& Nick);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void GPUUpdateScreenSize(AFINComputerGPU* GPU, FVector2D Size);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void GPUMouseEvent(AFINComputerGPUT1* GPU, int type, int x, int y, int btn);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void GPUKeyEvent(AFINComputerGPUT1* GPU, int type, int64 c, int64 code, int btn);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void GPUKeyCharEvent(AFINComputerGPUT1* GPU, const FString& c, int btn);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void GPUT2MouseEvent(AFINComputerGPUT2* GPU, int Type, FVector2D Position, int Modifiers);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void GPUT2MouseWheelEvent(AFINComputerGPUT2* GPU, FVector2D Position, float Delta, int Modifiers);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void GPUT2KeyEvent(AFINComputerGPUT2* GPU, int Type, int64 C, int64 Code, int Modifiers);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Computer|RCO")
	void GPUT2KeyCharEvent(AFINComputerGPUT2* GPU, const FString& C, int Modifiers);

	UFUNCTION(Server, Reliable)
	void CreateEEPROMState(UFGInventoryComponent* Inv, int SlotIdx);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void CopyDataItem(UFGInventoryComponent* InProviderInc, int InProviderIdx, UFGInventoryComponent* InFromInv, int InFromIdx, UFGInventoryComponent* InToInv, int InToIdx);

	UFUNCTION(Server, Reliable)
	void SetLabel(UFGInventoryComponent* Inventory, int32 Index, const FString& Label);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="FINLua|RCO")
	void SetTextEEPROMCode(class UFGInventoryComponent* Inventory, int32 Index, const FString& NewCode);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ItemStateUpdated(class UFGInventoryComponent* Inventory, int32 Index, FFIRInstancedStruct state);
};
