#pragma once

#include "CoreMinimal.h"
#include "FicsItVisualScript/Script/FIVSGraph.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Network/FINNetworkConnectionComponent.h"
#include "FINComponentUtility.generated.h"

class UNativeWidgetHost;
/**
 * Provides Utility functions for BP implemented network components
 */
UCLASS()
class FICSITNETWORKS_API UFINComponentUtility : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	UPROPERTY()
	UFIVSGraph* Graph;

	/**
	 * Trys to find the nearest network connector to the hit location of the the hit actor
	 *
	 * @param	hit		the hit result you want to use for the search
	 * @return	the neares network connector, nullptr if it was not able to find it
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility")
	static UFINNetworkConnectionComponent* GetNetworkConnectorFromHit(FHitResult hit);

	/**
	 * Sets the clipboard to the given string
	 *
	 * @param	str		the string you want to put into the clipboard
	 */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	static void ClipboardCopy(FString str);

	UFUNCTION(BlueprintCallable)
	static void TestFicsItVisualScript(UNativeWidgetHost* Widget);
};