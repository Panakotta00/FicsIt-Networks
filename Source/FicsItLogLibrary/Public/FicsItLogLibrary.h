#pragma once

#include "CoreMinimal.h"
#include "FILLogEntry.h"
#include "Modules/ModuleManager.h"
#include "FicsItLogLibrary.generated.h"

UENUM()
enum EFILLogOptions {
    FIL_Option_None,
    FIL_Option_Where,
    FIL_Option_Stack,
};

UCLASS()
class FICSITLOGLIBRARY_API UFILogLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
    static void Log(TEnumAsByte<EFILLogVerbosity> Verbosity, FString Message, TEnumAsByte<EFILLogOptions> Options = FIL_Option_Where);
};

class FFicsItLogLibraryModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
