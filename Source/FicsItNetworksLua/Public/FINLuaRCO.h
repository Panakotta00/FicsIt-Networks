#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FINLuaRCO.generated.h"

class UFGItemDescriptor;

UCLASS()
class FICSITNETWORKSLUA_API UFINLuaRCO : public UFGRemoteCallObject {
    GENERATED_BODY()
public:

private:
    UPROPERTY(Replicated)
    bool bDummy;
};
