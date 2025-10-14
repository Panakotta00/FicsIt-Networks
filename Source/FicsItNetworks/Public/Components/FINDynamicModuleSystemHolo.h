// 

#pragma once

#include "CoreMinimal.h"
#include "ModuleSystem/FINModuleSystemHolo.h"
#include "FINDynamicModuleSystemHolo.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINDynamicModuleSystemHolo : public AFINModuleSystemHolo {
	GENERATED_BODY()

	public:
		UPROPERTY(Replicated)
		int DynamicWidth = 1;

		UPROPERTY(Replicated)
		int DynamicHeight = 1;
	
		int OldDynamicHeight = 0;
		int OldDynamicWidth = 0;
		FVector Normal;
		bool bPlaced = false;

		UPROPERTY()
		TArray<USceneComponent*> Parts;

		UPROPERTY(Replicated)
		bool bFinished = false;

		UPROPERTY(EditDefaultsOnly)
		bool DynamicWidthAllowed;
		
		UPROPERTY(EditDefaultsOnly)
		bool DynamicHeightAllowed;

		UPROPERTY(EditDefaultsOnly)
		bool bBidirectionalPlacementAllowed;

		int MinWidth;
		
		int MinHeight;
		
		int MaxWidth;
		
		int MaxHeight;
		
		
		// Sets default values for this actor's properties
		AFINDynamicModuleSystemHolo();
		// Called when the game starts or when spawned
		virtual void BeginPlay() override;
		
		virtual void OnConstruction(const FTransform& Transform) override;

		virtual void CheckValidFloor() override;
		virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
		virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) override;

	protected:
		
		void ConstructParts();
		bool TrySnapToActor(const FHitResult& hitResult);
		void SetHologramLocationAndRotation(const FHitResult& hit);

	public:
		// Called every frame
		virtual void Tick(float DeltaTime) override;
		virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

		virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
		virtual void OnInvalidHitResult() override;
};

UINTERFACE(Blueprintable)
class UFINDynamicModuleBuildable : public UInterface {
	GENERATED_BODY()
};

class IFINDynamicModuleBuildable {
	GENERATED_BODY()

	public:
		UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SpawnComponents(int Width, int Height, AActor* Parent, USceneComponent* Attach, TArray<USceneComponent*>& OutParts);

		UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SetDynamicSize(int Width, int Height);

		UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void GetMinimumDynamicSize(int &MinWidth, int &MinHeight);
		
		UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void GetMaximumDynamicSize(int &MaxWidth, int &MaxHeight);

};
