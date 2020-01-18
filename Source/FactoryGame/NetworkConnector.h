// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FGSaveInterface.h"
#include "Buildables/FGBuildable.h"
#include "NetworkComponent.h"
#include "NetworkConnector.generated.h"


UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FACTORYGAME_API UNetworkConnector : public USceneComponent, public IFGSaveInterface, public INetworkComponent
{
	GENERATED_BODY()

public:
	UNetworkConnector();

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid id;

	UPROPERTY(BlueprintReadOnly, SaveGame)
		FString nick;

	UPROPERTY(SaveGame)
	bool idCreated;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int maxCables = -1;

	UFUNCTION(BlueprintCallable)
		void addConnection(UNetworkConnector* connector);

	UFUNCTION(BlueprintCallable)
		void removeConnection(UNetworkConnector* connector);

	UFUNCTION(BlueprintCallable)
		bool addCable(AFGBuildable* cable);

	UFUNCTION(BlueprintCallable)
		void removeCable(AFGBuildable* cable);

	UFUNCTION(BlueprintCallable)
		void addMerged(UObject* merged);

	UFUNCTION(BlueprintCallable)
		void addComponent(UObject* component);

	virtual bool ShouldSave_Implementation() const override;

	virtual FGuid getID_Implementation() const override;
	virtual FString getNick_Implementation() const override;
	virtual void setNick_Implementation(FString nick) override;
	virtual TArray<UObject*> getMerged_Implementation() const override;
	virtual TArray<UObject*> getConnected_Implementation() const override;
};
