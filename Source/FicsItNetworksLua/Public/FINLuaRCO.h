#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FINLuaRCO.generated.h"

class UFGItemDescriptor;

UCLASS()
class FICSITNETWORKSLUA_API UFINLuaRCO : public UFGRemoteCallObject {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Server, Reliable, Category="FINLua|RCO")
    void SetLuaEEPROMCode(class UFGInventoryComponent* Inventory, int32 Index, const FString& NewCode);

private:
    UPROPERTY(Replicated)
    bool bDummy;
};
