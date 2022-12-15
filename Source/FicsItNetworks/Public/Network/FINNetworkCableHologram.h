#pragma once

#include "FINNetworkAdapter.h"
#include "FINNetworkConnectionComponent.h"
#include "Hologram/FGBuildableHologram.h"
#include "Components/SplineMeshComponent.h"
#include "FINNetworkCableHologram.generated.h"

UENUM()
enum EFINNetworkCableHologramPlacementStepType {
	FIN_NOT_SNAPPED,
	FIN_CONNECTOR,
	FIN_ADAPTER,
	FIN_POLE,
	FIN_PLUG,
};

USTRUCT()
struct FICSITNETWORKS_API FFINCablePlacementStepInfo {
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<EFINNetworkCableHologramPlacementStepType> SnapType = FIN_NOT_SNAPPED;

	UPROPERTY()
	UObject* SnappedObj = nullptr;
	
	UPROPERTY()
	FFINAdapterSettings AdapterSettings;

	/** Location of the connector in world space */
	FVector GetConnectorPos() const;

	/** Rotation of the connector in world space */
	FRotator GetConnectorRot() const;
	
	AActor* GetActor() const;
};

UCLASS(Blueprintable)
class FICSITNETWORKS_API AFINNetworkCableHologram : public AFGBuildableHologram {
	GENERATED_BODY()
	friend FFINCablePlacementStepInfo;
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGRecipe> RecipePlug = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGRecipe> RecipePole = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGRecipe> RecipeAdapter = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	bool bEnablePole = true;

	UPROPERTY(EditDefaultsOnly)
	bool bEnablePlug = true;

	UPROPERTY(EditDefaultsOnly)
	bool bEnableAdapter = true;
	
	UPROPERTY()
	USplineMeshComponent* Cable = nullptr;

	UPROPERTY(Replicated)
	FFINCablePlacementStepInfo Snapped;

	UPROPERTY(Replicated)
	FFINCablePlacementStepInfo OldSnapped;

	UPROPERTY(Replicated)
	FFINCablePlacementStepInfo From;

	UPROPERTY()
	AFGBuildableHologram* PoleHologram1 = nullptr;

	UPROPERTY()
	AFGBuildableHologram* PlugHologram1 = nullptr;

	UPROPERTY()
	AFGBuildableHologram* PoleHologram2 = nullptr;

	UPROPERTY()
	AFGBuildableHologram* PlugHologram2 = nullptr;
	
	UPROPERTY()
	AFGBuildableHologram* AdapterHologram1 = nullptr;

	UPROPERTY()
	AFGBuildableHologram* AdapterHologram2 = nullptr;

	UPROPERTY()
	UFINNetworkConnectionComponent* Connector1Cache = nullptr;

	UPROPERTY()
	UFINNetworkConnectionComponent* Connector2Cache = nullptr;

	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	// End AActor

	// Begin FGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual AActor* Construct(TArray<AActor*>& childs, FNetConstructionID constructionID) override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool IsValidHitResult(const FHitResult& hit) const override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hit) override;
	virtual void OnInvalidHitResult() override;
	virtual bool IsChanged() const override;
	virtual USceneComponent* SetupComponent(USceneComponent* attachParent, UActorComponent* templateComponent, const FName& componentName) override;
	virtual void SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) override;
	// End FGBuildableHologram

	AFINNetworkCableHologram();
	~AFINNetworkCableHologram();

	void GetHolos(AFGBuildableHologram*& OutAdapterHolo, AFGBuildableHologram*& OutPlugHolo, AFGBuildableHologram*& OutPoleHolo);
	void UpdateCable();
	void UpdateChildHolos(const FHitResult& HitResult);
	void UpdateSnapping(const FHitResult& HitResult);
	void OnBeginSnap(FFINCablePlacementStepInfo a, bool isValid);
	void OnEndSnap(FFINCablePlacementStepInfo a);
	UFINNetworkConnectionComponent* SetupSnapped(FFINCablePlacementStepInfo s);
	void UpdateSnappingEffects();
	void UpdateMeshValidity(bool bValid);
	bool IsInSecondStep();

	/**
	 * Checks if the currently snapped object is valid
	 */
	bool IsSnappedValid();

protected:
	template<typename T>
	static T* GetDefaultBuildable_Static(AFGBuildableHologram* Holo) { return Holo->GetDefaultBuildable<T>(); }
	
	static UClass* GetBuildClass(AFGBuildableHologram* Holo) { return Holo->mBuildClass; }
};