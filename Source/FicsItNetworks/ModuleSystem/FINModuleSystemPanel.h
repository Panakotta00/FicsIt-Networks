#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FINModuleSystemModule.h"
#include "FGInventoryLibrary.h"
#include "FINModuleSystemPanel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINModuleDelegate, UObject*, module, bool, added);

/**
 * This component manages a collection of modules you can place onto it via the build gun.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class FICSITNETWORKS_API UFINModuleSystemPanel : public USceneComponent {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "ModuleSystem|Panel")
	int PanelWidth = 1;

	UPROPERTY(EditDefaultsOnly, Category = "ModuleSystem|Panel")
	int PanelHeight = 1;

	UPROPERTY(EditDefaultsOnly, Category = "ModuleSystem|Panel")
	TArray<UClass*> AllowedModules;
	
	UPROPERTY(BlueprintAssignable, Category = "ModuleSystem|Panel")
	FFINModuleDelegate OnModuleChanged;
	
	AActor*** grid;

	UFINModuleSystemPanel();
	~UFINModuleSystemPanel();

	// Begin UActorComponent
	virtual void PostLoad() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type reason) override;
	// End UActorComponent

	/**
	 * Returns the module at the given position.
	 * @return	the module or nullptr if none is set on the position
	 */
	UFUNCTION(BlueprintCallable, Category = "ModuleSystem|Panel")
	AActor* GetModule(int x, int y) const;

	/**
	 * Adds the given module to the given position and rotation.
	 * Also checks for clearance and validity of the placement.
	 * @return	true if it was able to place the module
	 */
	UFUNCTION(BlueprintCallable, Category = "ModuleSystem|Panel")
	bool AddModule(AActor* module, int x, int y, int rot);

	/**
	 * Removes the given module from the panel.
	 * !IMPORTANT! The module actor doesnt get destroyed, just the reference in the panel gets removed.
	 * @return true if it was able to remove the module from the panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModuleSystem|Panel")
	bool RemoveModule(AActor* module);

	/**
	 * Returns all the modules added to the panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModuleSystem|Panel")
	void GetModules(UPARAM(ref) TArray<AActor*>& out_modules);

	/**
	 * Returns the dismantle refund sum of all modules added to the panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModuleSystem|Panel")
	void GetDismantleRefund(UPARAM(ref) TArray<FInventoryStack>& out_refund);

	static void getModuleSpace(FVector loc, int rot, FVector msize, FVector& min, FVector& max);
};