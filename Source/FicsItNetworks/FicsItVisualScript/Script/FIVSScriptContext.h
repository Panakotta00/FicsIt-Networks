#pragma once

#include "CoreMinimal.h"
#include "FicsItNetworks/Network/FINNetworkTrace.h"
#include "FIVSScriptContext.generated.h"

class UFINStruct;
class UFINClass;

UINTERFACE()
class UFIVSScriptContext_Interface : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKS_API IFIVSScriptContext_Interface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent)
	void GetRelevantObjects(TArray<FFINNetworkTrace>& OutObjects);

	UFUNCTION(BlueprintNativeEvent)
	void GetRelevantClasses(TArray<UFINClass*>& OutClasses);

	UFUNCTION(BlueprintNativeEvent)
	void GetRelevantStructs(TArray<UFINStruct*>& OutStructs);
};