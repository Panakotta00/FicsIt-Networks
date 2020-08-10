#pragma once
#include "FGBuildable.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"

#include "FINVehicleScanner.generated.h"

class AFGVehicle;

UCLASS()
class AFINVehicleScanner : public AFGBuildable, public IFINSignalSender {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector;
	
	UPROPERTY(EditAnywhere)
	UFGColoredInstanceMeshProxy* StaticMesh;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* LampMesh;

	UPROPERTY(EditAnywhere)
	UBoxComponent* VehicleCollision;

	UPROPERTY(SaveGame)
	TSet<FFINNetworkTrace> SignalListeners;
	
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FLinearColor ScanColor = FLinearColor(0,0,1);

	UPROPERTY(BlueprintReadOnly, SaveGame)
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
	virtual void AddListener_Implementation(FFINNetworkTrace listener) override;
	virtual void RemoveListener_Implementation(FFINNetworkTrace listener) override;
	virtual TSet<FFINNetworkTrace> GetListeners_Implementation() override;
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender

	UFUNCTION(BlueprintNativeEvent)
	void UpdateColor();

	UFUNCTION()
	void netFunc_setColor(float r, float g, float b, float e);

	UFUNCTION()
	AFGVehicle* netFunc_getLastVehicle();

	UFUNCTION(BlueprintNativeEvent)
	void netSig_OnVehicleEnter(AFGVehicle* Vehicle);

	UFUNCTION(BlueprintNativeEvent)
	void netSig_OnVehicleExit(AFGVehicle* Vehicle);
};
