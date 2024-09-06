#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FINLuaRCO.generated.h"

UCLASS()
class FICSITNETWORKSLUA_API UFINLuaRCO : public UFGRemoteCallObject {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="FINLua|RCO")
    void SetLuaEEPROMCode(AFINStateEEPROMLua* LuaEEPROMState, const FString& NewCode);

private:
    UPROPERTY(Replicated)
    bool bDummy;
};
