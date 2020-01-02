// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ModuleSystemPanel.h"
#include "ModuleSystemModule.generated.h"

UINTERFACE(MinimalAPI)
class UModuleSystemModule : public UInterface
{
	GENERATED_BODY()
};

class FACTORYGAME_API IModuleSystemModule
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void getModuleSize(int& width, int& height) const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void setPanel(UModuleSystemPanel* panel, int x, int y, int rot);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		FName getName() const;
public:
};
