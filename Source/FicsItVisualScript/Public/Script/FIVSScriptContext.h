#pragma once

#include "CoreMinimal.h"
#include "FIRTrace.h"
#include "Interface.h"
#include "FIVSScriptContext.generated.h"

struct FFIVSLuaCompilerContext;
class UFINStruct;
class UFINClass;

DECLARE_DELEGATE_OneParam(FFIVSOnScriptCompiled, const FFIVSLuaCompilerContext&);

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

	virtual FFIVSOnScriptCompiled& GetOnScriptCompiledEvent() = 0;
};