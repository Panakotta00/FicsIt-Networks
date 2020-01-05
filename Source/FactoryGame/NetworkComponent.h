// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NetworkCircuit.h"
#include "NetworkComponent.generated.h"

USTRUCT()
struct FTestStruc {
	GENERATED_BODY()

		int test;
};

UENUM()
enum ETestEnum {
	Test1,
	Test2,
	Test3
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNetworkComponent : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FACTORYGAME_API INetworkComponent
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		FGuid getID() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		TArray<UObject*> getMerged() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		TArray<UObject*> getConnected() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		UObject* findComponent(FGuid id) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		UNetworkCircuit* getCircuit() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void setCircuit(UNetworkCircuit* circuit);
};
