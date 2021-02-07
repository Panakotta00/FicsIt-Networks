#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FGInventoryLibrary.h"

#include "FINModuleSystemPanel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINModuleDelegate, UObject*, module, bool, added);

/**
 * This component manages a collection of modules you can place onto it via the build gun.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class FICSITNETWORKS_API UFINModuleSystemPanel : public USceneComponent, public IFGSaveInterface {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "ModuleSystem|Panel")
	int PanelWidth = 1;

	UPROPERTY(EditDefaultsOnly, Category = "ModuleSystem|Panel")
	int PanelHeight = 1;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "ModuleSystem|Panel")
	TArray<UClass*> AllowedModules;
	
	UPROPERTY(BlueprintAssignable, Category = "ModuleSystem|Panel")
	FFINModuleDelegate OnModuleChanged;

	UPROPERTY(Replicated)
	TArray<TWeakObjectPtr<UObject>> Grid;
	
	UFINModuleSystemPanel();
	~UFINModuleSystemPanel();

	// Begin UObject
	void Serialize(FArchive& Ar) override;
	// End UObject

	// Begin UActorComponent
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	// End UActorComponent

	// Begin IFGSaveInterface
	bool ShouldSave_Implementation() const override;
	virtual void GatherDependencies_Implementation(TArray< UObject* >& out_dependentObjects) override;
	// End IFGSaveInterface

	/**
	 * Returns the module at the given position.
	 * @return	the module or nullptr if none is set on the position
	 */
	UFUNCTION(BlueprintCallable, Category = "ModuleSystem|Panel")
	AActor* GetModule(int X, int Y) const;

	/**
	 * Adds the given module to the given position and rotation.
	 * Also checks for clearance and validity of the placement.
	 * @return	true if it was able to place the module
	 */
	UFUNCTION(BlueprintCallable, Category = "ModuleSystem|Panel")
	bool AddModule(AActor* Module, int X, int Y, int Rot);

	/**
	 * Removes the given module from the panel.
	 * !IMPORTANT! The module actor does not get destroyed, just the reference in the panel gets removed.
	 * @return true if it was able to remove the module from the panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModuleSystem|Panel")
	bool RemoveModule(AActor* Module);

	/**
	 * Returns all the modules added to the panel.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "ModuleSystem|Panel")
	void GetModules(UPARAM(ref) TArray<AActor*>& out_modules) const;

	/**
	 * Returns the dismantle refund sum of all modules added to the panel.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "ModuleSystem|Panel")
	void GetDismantleRefund(UPARAM(ref) TArray<FInventoryStack>& out_refund) const;

	/**
	 * Check if it should allocate the grid cache
	 */
	void SetupGrid();

	/**
	 * Get Grid Slot at given location
	 */
	const TWeakObjectPtr<UObject>& GetGridSlot(int x, int y) const;
	TWeakObjectPtr<UObject>& GetGridSlot(int x, int y);

	static void GetModuleSpace(const FVector& Loc, int Rot, const FVector& MSize, FVector& OutMin, FVector& OutMax);
};