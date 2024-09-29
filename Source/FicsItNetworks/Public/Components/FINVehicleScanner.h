#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "Signals/FINSignalSender.h"
#include "FINVehicleScanner.generated.h"

class AFGVehicle;

UCLASS()
class AFINVehicleScanner : public AFGBuildable, public IFINSignalSender {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	class UFINAdvancedNetworkConnectionComponent* NetworkConnector;
	
	UPROPERTY(EditAnywhere)
	UFGColoredInstanceMeshProxy* StaticMesh;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* LampMesh;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* VehicleCollision;

	UPROPERTY(BlueprintReadOnly, SaveGame, Replicated)
	FLinearColor ScanColor = FLinearColor(0,0,1);

	UPROPERTY(BlueprintReadOnly, SaveGame, Replicated)
	float Intensity;

	UPROPERTY(SaveGame)
	AFGVehicle* LastVehicle;
	
	UPROPERTY()
	UMaterialInstanceDynamic* LightMaterialInstance;

	UPROPERTY()
	bool bColorChanged = false;

	AFINVehicleScanner();

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	// End AActor

	// Begin IFINSignalSender
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender

	UFUNCTION(NetMulticast, Reliable)
	void Client_OnColorChanged();

	UFUNCTION(BlueprintNativeEvent)
	void UpdateColor();

	UFUNCTION()
	void netClass_Meta(FString& InternalName, FText& DisplayName) {
		InternalName = TEXT("VehicleScanner");
		DisplayName = FText::FromString(TEXT("Vehicle Scanner"));
	}

	UFUNCTION()
	void netFunc_setColor(float r, float g, float b, float e);
	UFUNCTION()
    void netFuncMeta_setColor(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setColor";
		DisplayName = FText::FromString("Set Color");
		Description = FText::FromString("Allows to change the color and light intensity of the scanner.");
		ParameterInternalNames.Add("r");
		ParameterDisplayNames.Add(FText::FromString("Red"));
		ParameterDescriptions.Add(FText::FromString("The red part of the color in which the scanner glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("g");
		ParameterDisplayNames.Add(FText::FromString("Green"));
		ParameterDescriptions.Add(FText::FromString("The green part of the color in which the scanner glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("b");
		ParameterDisplayNames.Add(FText::FromString("Blue"));
		ParameterDescriptions.Add(FText::FromString("The blue part of the color in which the scanner glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("e");
		ParameterDisplayNames.Add(FText::FromString("Emissive"));
		ParameterDescriptions.Add(FText::FromString("The light intensity of the scanner. (0.0 - 5.0)"));
		Runtime = 2;
	}

	UFUNCTION()
    void netFunc_getColor(float& r, float& g, float& b, float& e);
	UFUNCTION()
    void netFuncMeta_getColor(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getColor";
		DisplayName = FText::FromString("Get Color");
		Description = FText::FromString("Allows to get the color and light intensity of the scanner.");
		ParameterInternalNames.Add("r");
		ParameterDisplayNames.Add(FText::FromString("Red"));
		ParameterDescriptions.Add(FText::FromString("The red part of the color in which the scanner glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("g");
		ParameterDisplayNames.Add(FText::FromString("Green"));
		ParameterDescriptions.Add(FText::FromString("The green part of the color in which the scanner glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("b");
		ParameterDisplayNames.Add(FText::FromString("Blue"));
		ParameterDescriptions.Add(FText::FromString("The blue part of the color in which the scanner glows. (0.0 - 1.0)"));
		ParameterInternalNames.Add("e");
		ParameterDisplayNames.Add(FText::FromString("Emissive"));
		ParameterDescriptions.Add(FText::FromString("The light intensity of the scanner. (0.0 - 5.0)"));
		Runtime = 1;
	}

	UFUNCTION()
	AFGVehicle* netFunc_getLastVehicle();
	UFUNCTION()
    void netFuncMeta_getLastVehicle(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getLastVehicle";
		DisplayName = FText::FromString("Get Last Vehicle");
		Description = FText::FromString("Returns the last vehicle that entered the scanner.");
		ParameterInternalNames.Add("vehicle");
		ParameterDisplayNames.Add(FText::FromString("Vehicle"));
		ParameterDescriptions.Add(FText::FromString("The vehicle that entered the scanner. null if it has already left the scanner."));
		Runtime = 1;
	}

	UFUNCTION(BlueprintNativeEvent)
	void netSig_OnVehicleEnter(AFGVehicle* Vehicle);
	UFUNCTION()
    void netSigMeta_OnVehicleEnter(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnVehicleEnter";
		DisplayName = FText::FromString("On Vehicle Enter");
		Description = FText::FromString("Triggers when a vehicle enters the scanner.");
		ParameterInternalNames.Add("vehicle");
		ParameterDisplayNames.Add(FText::FromString("Vehicle"));
		ParameterDescriptions.Add(FText::FromString("The vehicle that entered the scanner."));
		Runtime = 1;
	}

	UFUNCTION(BlueprintNativeEvent)
	void netSig_OnVehicleExit(AFGVehicle* Vehicle);
	UFUNCTION()
    void netSigMeta_OnVehicleExit(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnVehicleExit";
		DisplayName = FText::FromString("On Vehicle Exit");
		Description = FText::FromString("Triggers when a vehicle leaves the scanner.");
		ParameterInternalNames.Add("vehicle");
		ParameterDisplayNames.Add(FText::FromString("Vehicle"));
		ParameterDescriptions.Add(FText::FromString("The vehicle that left the scanner."));
		Runtime = 1;
	}
};
