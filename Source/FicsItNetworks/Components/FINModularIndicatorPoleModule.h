#pragma once

#include "Buildables/FGBuildable.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
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
	UStaticMeshComponent* Indicator;

	UPROPERTY(SaveGame, Replicated)
	FLinearColor IndicatorColor = FLinearColor::Black;

	UPROPERTY(SaveGame, Replicated)
	float EmissiveStrength = 0.1;
	
	UPROPERTY()
	UMaterialInstanceDynamic* IndicatorMaterialInstance = nullptr;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Poles;

	
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
	UFUNCTION(NetMulticast, Reliable)
	void UpdateEmessive();
	
	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName, TMap<FString, FString>& PropertyInternalNames, TMap<FString, FText>& PropertyDisplayNames, TMap<FString, FText>& PropertyDescriptions, TMap<FString, int32>& PropertyRuntimes) {
		InternalName = TEXT("IndicatorPole");
		DisplayName = FText::FromString(TEXT("Indicator Pole"));
	}

	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void SetColor(float r, float g, float b, float e);

	
	UFUNCTION()
	void netFunc_setColor(float r, float g, float b, float e);
	UFUNCTION()
    void netFuncMeta_setColor(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setColor";
		DisplayName = FText::FromString("Set Color");
		Description = FText::FromString("Allows to change the color and light intensity of the indicator lamp.");
		ParameterInternalNames.Add("r");
		ParameterDisplayNames.Add(FText::FromString("Red"));
		ParameterDescriptions.Add(FText::FromString("The red part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("g");
		ParameterDisplayNames.Add(FText::FromString("Green"));
		ParameterDescriptions.Add(FText::FromString("The green part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("b");
		ParameterDisplayNames.Add(FText::FromString("Blue"));
		ParameterDescriptions.Add(FText::FromString("The blue part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("e");
		ParameterDisplayNames.Add(FText::FromString("Emissive"));
		ParameterDescriptions.Add(FText::FromString("The light intensity of the pole. (0.0 - 5.0)"));
		Runtime = 2;
	}
	
	UFUNCTION()
	void netFunc_getColor(float& r, float& g, float& b, float& e);
	UFUNCTION()
    void netFuncMeta_getColor(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getColor";
		DisplayName = FText::FromString("Get Color");
		Description = FText::FromString("Allows to get the color and light intensity of the indicator lamp.");
		ParameterInternalNames.Add("r");
		ParameterDisplayNames.Add(FText::FromString("Red"));
		ParameterDescriptions.Add(FText::FromString("The red part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("g");
		ParameterDisplayNames.Add(FText::FromString("Green"));
		ParameterDescriptions.Add(FText::FromString("The green part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("b");
		ParameterDisplayNames.Add(FText::FromString("Blue"));
		ParameterDescriptions.Add(FText::FromString("The blue part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("e");
		ParameterDisplayNames.Add(FText::FromString("Emissive"));
		ParameterDescriptions.Add(FText::FromString("The light intensity of the pole. (0.0 - 5.0)"));
		Runtime = 1;
	}
	
	UFUNCTION()
	void netSig_ColorChanged(float r, float g, float b, float e);
	UFUNCTION()
    void netSigMeta_ColorChanged(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "ColorChanged";
		DisplayName = FText::FromString("Color Changed");
		Description = FText::FromString("Triggers when the color of the indicator pole changes.");
		ParameterInternalNames.Add("r");
		ParameterDisplayNames.Add(FText::FromString("Red"));
		ParameterDescriptions.Add(FText::FromString("The red part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("g");
		ParameterDisplayNames.Add(FText::FromString("Green"));
		ParameterDescriptions.Add(FText::FromString("The green part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("b");
		ParameterDisplayNames.Add(FText::FromString("Blue"));
		ParameterDescriptions.Add(FText::FromString("The blue part of the color in which the light glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("e");
		ParameterDisplayNames.Add(FText::FromString("Emissive"));
		ParameterDescriptions.Add(FText::FromString("The light intensity of the pole. (0.0 - 5.0)"));
		Runtime = 1;
	}
	
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
