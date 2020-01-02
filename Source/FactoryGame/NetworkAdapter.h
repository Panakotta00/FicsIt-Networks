// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Buildables/FGBuildableFactory.h"
#include "NetworkConnector.h"
#include "NetworkAdapter.generated.h"

UCLASS(BlueprintType)
class FACTORYGAME_API ANetworkAdapter : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
		AFGBuildableFactory* parent;

	UPROPERTY(BlueprintReadWrite)
		UNetworkConnector* connector;
	
	UPROPERTY(BlueprintReadOnly)
		UChildActorComponent* attachment;

	// Sets default values for this actor's properties
	ANetworkAdapter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
