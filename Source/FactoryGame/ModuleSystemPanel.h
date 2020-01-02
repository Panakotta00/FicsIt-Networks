// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Inventory.h"
#include "ModuleSystemPanel.generated.h"


UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FACTORYGAME_API UModuleSystemPanel : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
		int modulePanelWidth;
	UPROPERTY(EditDefaultsOnly)
		int modulePanelHeight;
	UPROPERTY(EditDefaultsOnly, meta = (MustImplement = "ModuleSystemModule"))
		TArray<TSubclassOf<AActor>> allowedModules;

	UModuleSystemPanel();

	UFUNCTION(BlueprintCallable)
		void addModule(AActor* module, int x, int y, int rot);

	UFUNCTION(BlueprintCallable)
		void removeModule(AActor* module);

	UFUNCTION(BlueprintCallable)
		AActor* getModule(int x, int y) const;

	UFUNCTION(BlueprintCallable)
		void getModules(UPARAM(ref) TArray< AActor* >& out_modules);

	UFUNCTION(BlueprintCallable, Category = "Dismantle")
		void GetPanelDismantleRefund(UPARAM(ref) TArray< FInventoryStack >& out_refund);

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
