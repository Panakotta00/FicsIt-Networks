#pragma once

#include "FGBuildable.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "FINIndicatorPole.generated.h"

UCLASS()
class AFINIndicatorPole : public AFGBuildable {
	GENERATED_BODY()
	
public:
	UPROPERTY(SaveGame)
	AFINIndicatorPole* TopConnected = nullptr;

	UPROPERTY(SaveGame)
	AFINIndicatorPole* BottomConnected = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Replicated)
	int Height = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* Connector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* Indicator;

	UPROPERTY(SaveGame, Replicated)
	FLinearColor IndicatorColor = FLinearColor::Black;

	UPROPERTY(SaveGame, Replicated)
	float EmessiveStrength = 5.0;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* ShortPole = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* LongPole = nullptr;
	
	UPROPERTY()
	UMaterialInstanceDynamic* IndicatorInstance = nullptr;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Poles;

	UPROPERTY()
	bool bHasChanged = false;
	
	AFINIndicatorPole();
	
	// Begin AActor
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFGDismantleInterface
	virtual int32 GetDismantleRefundReturnsMultiplier() const override;
	// End IFGDismantleInterface

	/**
	 * Spawns all the pole static meshes
	 */
	void CreatePole();

	/**
	 * Updates the material paramteres of the dynamic material instance
	 * to the variables.
	 */
	UFUNCTION(NetMulticast, Reliable)
	void UpdateEmessive();

	UFUNCTION()
	void netFunc_setColor(float r, float g, float b, float e);
	UFUNCTION()
	void netFunc_getColor(float& r, float& g, float& b, float& e);
	UFUNCTION()
	void netSig_ColorChanged(float r, float g, float b, float e);
	UFUNCTION()
	AFINIndicatorPole* netFunc_getTopPole();
	UFUNCTION()
	AFINIndicatorPole* netFunc_getBottomPole();
};
