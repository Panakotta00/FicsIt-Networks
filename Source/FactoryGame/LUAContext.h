// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LuaContext.generated.h"

UENUM(BlueprintType)
enum ELuaState {
	HALTED = 0,
	RUNNING = 1,
	FINISHED = 2,
	CRASHED = 3
};

/**
 * 
 */
UCLASS(BlueprintType)
class FACTORYGAME_API ULuaContext : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int memory;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString code;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString log;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString exception;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<ELuaState> state;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UObject* controller;

	UFUNCTION(BlueprintCallable)
		void ExecSteps(int32 count);

	UFUNCTION(BlueprintCallable)
		void Reset();

	UFUNCTION(BlueprintCallable)
		void SetCode(FString newCode);

	UFUNCTION(BlueprintCallable/*, meta = (CustomStructureParam = "any")*/)
		void signalSlot(FString event, FString name);
};
