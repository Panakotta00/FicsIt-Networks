#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine.h"

#include "RozeBlueprintLibrary.generated.h"

UCLASS()
class URozeBlueprintLibrary : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()
	//GENERATED_UCLASS_BODY()
	
    /** Starts an analytics session without any custom attributes specified */
    UFUNCTION(BlueprintCallable, Category="Utilities")
    static FString ExpandPath(FString folder, FString file, FString extension);
};
