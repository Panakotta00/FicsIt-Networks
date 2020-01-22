// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/FGEquipment.h"
#include "FileSystem.h"
#include "FGInventoryComponent.h"
#include "Equip_FileSystem.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class FACTORYGAME_API AEquip_FileSystem : public AFGEquipment
{
	GENERATED_BODY()
	
public:
	AEquip_FileSystem();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UFileSystem* FileSystem;

	UFUNCTION(BlueprintCallable)
		static AEquip_FileSystem* createState(int capacity, UFGInventoryComponent* inventory, int slotIndex);

};
