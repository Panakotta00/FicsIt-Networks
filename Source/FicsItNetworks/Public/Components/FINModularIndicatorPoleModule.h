#pragma once

#include "Buildables/FGBuildable.h"
#include "FINModularIndicatorPoleModule.generated.h"

UCLASS()
class AFINModularIndicatorPoleModule : public AFGBuildable {
	GENERATED_BODY()
	
public:
	UPROPERTY(SaveGame, Replicated)
	AFINModularIndicatorPoleModule* NextChild = nullptr;

	UPROPERTY(SaveGame)
	AFGBuildable* Parent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector ModuleConnectionPoint;

	UPROPERTY()
	bool bHasChanged = false;
	
	AFINModularIndicatorPoleModule();
	
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
	
	virtual void GetChildDismantleActors_Implementation(TArray<AActor*>& out_ChildDismantleActors) const override;

	/**
	 * Spawns all the pole static meshes
	 */

	/**
	 * Updates the material paramteres of the dynamic material instance
	 * to the variables.
	 */
	
	
	UFUNCTION()
	AFINModularIndicatorPoleModule* netFunc_getNext();
	UFUNCTION()
    void netFuncMeta_getNext(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getNext";
		DisplayName = FText::FromString("Get Next");
		Description = FText::FromString("Returns the next pole module if any.");
		ParameterInternalNames.Add("next");
		ParameterDisplayNames.Add(FText::FromString("Next module"));
		ParameterDescriptions.Add(FText::FromString("The next module in this chain."));
		Runtime = 1;
	}

	UFUNCTION()
	AFGBuildable* netFunc_getPrevious();
	UFUNCTION()
    void netFuncMeta_getPrevious(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getPrevious";
		DisplayName = FText::FromString("Get Previous");
		Description = FText::FromString("Gets the previous module or the base mount if this called from the last module.");
		ParameterInternalNames.Add("previous");
		ParameterDisplayNames.Add(FText::FromString("Previous module"));
		ParameterDescriptions.Add(FText::FromString("The previous module or base mount."));
		Runtime = 1;
	}
};
