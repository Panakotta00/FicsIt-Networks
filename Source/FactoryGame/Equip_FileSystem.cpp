// Fill out your copyright notice in the Description page of Project Settings.

#include "Equip_FileSystem.h"

AEquip_FileSystem::AEquip_FileSystem() {
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	FileSystem = CreateDefaultSubobject<UFileSystem>("FileSystem");

	for (auto fi = TFieldIterator<UProperty>(StaticClass()); fi; ++fi) {
		UE_LOG(LogTemp, Warning, TEXT("F: %s %i %i"), *fi->GetName(), fi->GetOffset_ForInternal(), fi->GetPropertyFlags());
	}
}

AEquip_FileSystem* AEquip_FileSystem::createState(int capacity, UFGInventoryComponent* inventory, int slotIndex) {
	return nullptr;
}