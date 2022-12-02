#pragma once

#include "Buildables/FGBuildable.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
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
	UFGColoredInstanceMeshProxy* Indicator;

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
	bool bHasChanged = true;

	int ChangeCount = 0;
	
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

	virtual void OnBuildEffectFinished() override;

	/**
	 * Spawns all the pole static meshes
	 */
	void CreatePole();

	/**
	 * Updates the material paramteres of the dynamic material instance
	 * to the variables.
	 */
	UFUNCTION()
	void UpdateEmessive();
	UFUNCTION(NetMulticast, Reliable)
	void UpdateEmessive_Net();

	
	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName, TMap<FString, FString>& PropertyInternalNames, TMap<FString, FText>& PropertyDisplayNames, TMap<FString, FText>& PropertyDescriptions, TMap<FString, int32>& PropertyRuntimes) {
		InternalName = TEXT("IndicatorPole");
		DisplayName = FText::FromString(TEXT("Indicator Pole"));
	}
	
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
	AFINIndicatorPole* netFunc_getTopPole();
	UFUNCTION()
    void netFuncMeta_getTopPole(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getTopPole";
		DisplayName = FText::FromString("Get Top Pole");
		Description = FText::FromString("Allows to get the pole placed on top of this pole.");
		ParameterInternalNames.Add("topPole");
		ParameterDisplayNames.Add(FText::FromString("Top Pole"));
		ParameterDescriptions.Add(FText::FromString("The pole placed on top of this pole."));
		Runtime = 1;
	}

	UFUNCTION()
	AFINIndicatorPole* netFunc_getBottomPole();
	UFUNCTION()
    void netFuncMeta_getBottomePole(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getBottomPole";
		DisplayName = FText::FromString("Get Bottom Pole");
		Description = FText::FromString("Allows to get the pole on which this pole is placed on-top of.");
		ParameterInternalNames.Add("bottomPole");
		ParameterDisplayNames.Add(FText::FromString("Bottom Pole"));
		ParameterDescriptions.Add(FText::FromString("The pole on which this pole is placed on-top of."));
		Runtime = 1;
	}
};
