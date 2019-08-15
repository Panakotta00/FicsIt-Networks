// Copyright 2016-2018 Coffee Stain Studios. All Rights Reserved.

#pragma once
#include "Array.h"
#include "UObject/Class.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FGAttentionPingActor.generated.h"

UCLASS( Blueprintable )
class FACTORYGAME_API AFGAttentionPingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFGAttentionPingActor();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

	/** Set the slot idx for the player that spawned this actor */
	void SetPlayerSlotIdx( int32 slotIdx );

	/** Slot index of the player that spawned this actor */
	UFUNCTION( BlueprintPure, Category = "Attention Ping" )
	FORCEINLINE int32 GetPlayerSlotIdx() const { return mPlayerSlotIdx; }

	UFUNCTION()
	void OnRep_PlayerSlotIdx();

	/** Called when we have the slot idx replicated so now we can spawn the effects */
	UFUNCTION( BlueprintImplementableEvent, Category = "Attention Ping" )
	void SpawnAttentionPingEffects();

protected:

	/** Who spawned this ping? */
	UPROPERTY( ReplicatedUsing = OnRep_PlayerSlotIdx )
	int32 mPlayerSlotIdx;	
	
};
