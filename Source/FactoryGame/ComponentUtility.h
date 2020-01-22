// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGPowerConnectionComponent.h"
#include "NetworkConnector.h"
#include "Sound/SoundWave.h"
#include "ComponentUtility.generated.h"

/**
 * 
 */
UCLASS()
class FACTORYGAME_API UComponentUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static void connectPower(UFGPowerConnectionComponent* comp1, UFGPowerConnectionComponent* comp2);
	UFUNCTION(BlueprintCallable)
	static void disconnectPower(UFGPowerConnectionComponent* comp1, UFGPowerConnectionComponent* comp2);

	UFUNCTION(BlueprintPure)
	static UNetworkConnector* getNetworkConnectorFromHit(FHitResult hit);

	UFUNCTION(BlueprintCallable)
	static void clipboardCopy(FString str);

	UFUNCTION(BlueprintCallable)
	static void setAllowUsing(bool newUsing);

	UFUNCTION(BlueprintCallable)
		static USoundWave* loadSoundFromFile(FString file);

	UFUNCTION(BlueprintCallable)
		static void dumpObject(UObject* object);
};
