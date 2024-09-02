#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "FINComponentUtility.generated.h"

/**
 * Provides Utility functions for BP implemented network components
 */
UCLASS()
class FICSITNETWORKS_API UFINComponentUtility : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	/**
	 * Sets the clipboard to the given string
	 *
	 * @param	str		the string you want to put into the clipboard
	 */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	static void ClipboardCopy(FString str);
};