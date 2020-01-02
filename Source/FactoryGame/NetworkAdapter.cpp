// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkAdapter.h"
#include "FGPowerInfoComponent.h"
#include "Hologram/FGBuildableHologram.h"
#include "FGFactoryConnectionComponent.h"

// Sets default values
ANetworkAdapter::ANetworkAdapter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UObjectProperty;
	FActorSpawnParameters;
	UWorld* w = nullptr;
}

// Called when the game starts or when spawned
void ANetworkAdapter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANetworkAdapter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

