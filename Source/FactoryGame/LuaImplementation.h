// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LuaContext.h"
#include "Sound/SoundWave.h"
#include "LuaImplementation.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULuaImplementation : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FACTORYGAME_API ILuaImplementation
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void luaSetup(ULuaContext* ctx);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void luaAddSignalListener(ULuaContext* ctx);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void luaRemoveSignalListener(ULuaContext* ctx);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		TArray<ULuaContext*> luaGetSignalListeners();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		bool luaIsReachableFrom(UObject* listener);
};
