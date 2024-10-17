#pragma once

#include "CoreMinimal.h"
#include "FIVSScriptContext.generated.h"

class UFINStruct;
class UFINClass;

UINTERFACE()
class UFIVSScriptContext_Interface : public UInterface {
	GENERATED_BODY()
};

class FICSITVISUALSCRIPT_API IFIVSScriptContext_Interface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent)
	void GetRelevantObjects(TArray<FFIRTrace>& OutObjects);

	UFUNCTION(BlueprintNativeEvent)
	void GetRelevantClasses(TArray<UFIRClass*>& OutClasses);

	UFUNCTION(BlueprintNativeEvent)
	void GetRelevantStructs(TArray<UFIRStruct*>& OutStructs);
};