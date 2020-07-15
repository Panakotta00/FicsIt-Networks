#pragma once

#include "FGBuildable.h"
#include "Network/FINNetworkConnector.h"
#include "FINIndicatorPole.generated.h"

UCLASS()
class AFINIndicatorPole : public AFGBuildable {
	GENERATED_BODY()
	
public:
	UPROPERTY(SaveGame)
	TWeakObjectPtr<AFINIndicatorPole> TopConnected = nullptr;

	UPROPERTY(SaveGame)
	TWeakObjectPtr<AFINIndicatorPole> BottomConnected = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame)
	int Height = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame)
	UFINNetworkConnector* Connector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	UProxyInstancedStaticMeshComponent* Indicator;

	UPROPERTY(SaveGame)
	FLinearColor IndicatorColor = FLinearColor::Black;

	UPROPERTY(SaveGame)
	float EmessiveStrength = 5.0;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* ShortPole = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* LongPole = nullptr;
	
	UPROPERTY()
	UMaterialInstanceDynamic* IndicatorInstance = nullptr;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Poles;
	
	AFINIndicatorPole();

	// Begin AActor
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
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
