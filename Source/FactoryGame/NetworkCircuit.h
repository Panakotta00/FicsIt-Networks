// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject.h"
#include "NetworkCircuit.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class FACTORYGAME_API UNetworkCircuit : public UObject
{
	GENERATED_BODY()

public:
	UNetworkCircuit();
	~UNetworkCircuit();

	UFUNCTION(BlueprintCallable)
		TArray<UObject*> getComponents();
};
