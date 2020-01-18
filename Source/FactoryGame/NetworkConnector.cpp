// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkConnector.h"
#include "Hologram/FGWireHologram.h"
#include "RichTextBlock.h"
#include "FGBlueprintFunctionLibrary.h"

// Sets default values for this component's properties
UNetworkConnector::UNetworkConnector()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//IsValid(this);
	//UFGBlueprintFunctionLibrary::ShowOutline();
	FTransform t;

	// ...
}

void UNetworkConnector::addConnection(UNetworkConnector* connector) {}
void UNetworkConnector::removeConnection(UNetworkConnector* connector) {}
bool UNetworkConnector::addCable(AFGBuildable * cable) { return false; }
void UNetworkConnector::removeCable(AFGBuildable * cable) {}
void UNetworkConnector::addMerged(UObject * merged) {}
void UNetworkConnector::addComponent(UObject * component) {}
bool UNetworkConnector::ShouldSave_Implementation() const { return false; }
FGuid UNetworkConnector::getID_Implementation() const { return FGuid(); }
FString UNetworkConnector::getNick_Implementation() const { return ""; }
void UNetworkConnector::setNick_Implementation(FString nick) {}
TArray<UObject*> UNetworkConnector::getMerged_Implementation() const { return TArray<UObject*>();  };
TArray<UObject*> UNetworkConnector::getConnected_Implementation() const { return TArray<UObject*>(); };