// Fill out your copyright notice in the Description page of Project Settings.

#include "ComponentUtility.h"

void UComponentUtility::connectPower(UFGPowerConnectionComponent* comp1, UFGPowerConnectionComponent* comp2) {}
void UComponentUtility::disconnectPower(UFGPowerConnectionComponent* comp1, UFGPowerConnectionComponent* comp2) {}

UNetworkConnector * UComponentUtility::getNetworkConnectorFromHit(FHitResult hit) {
	return nullptr;
	FTransform t;
	t.GetRotation().Rotator();
}

void UComponentUtility::clipboardCopy(FString str) {}
void UComponentUtility::setAllowUsing(bool newUsing) {}
USoundWave* UComponentUtility::loadSoundFromFile(FString file) {return nullptr;}