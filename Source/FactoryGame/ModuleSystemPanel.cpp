// Fill out your copyright notice in the Description page of Project Settings.

#include "ModuleSystemPanel.h"

// Sets default values for this component's properties
UModuleSystemPanel::UModuleSystemPanel()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UModuleSystemPanel::addModule(AActor* module, int x, int y, int rot) {

}

void UModuleSystemPanel::removeModule(AActor* module) {

}

AActor* UModuleSystemPanel::getModule(int x, int y) const { return nullptr; }

void UModuleSystemPanel::getModules(TArray< AActor* >& out_modules) { }

void UModuleSystemPanel::GetPanelDismantleRefund(TArray< FInventoryStack >& out_refund) { }

// Called when the game starts
void UModuleSystemPanel::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UModuleSystemPanel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

