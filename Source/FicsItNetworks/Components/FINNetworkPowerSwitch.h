#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "FGPowerConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Network/FINNetworkCustomType.h"

#include "FINNetworkPowerSwitch.generated.h"

UCLASS()
class AFINNetworkPowerSwitch : public AFGBuildable, public IFINNetworkCustomType {
	GENERATED_BODY()

public:
	bool bConnectedHasChanged;

	UPROPERTY(BlueprintReadOnly, SaveGame, Replicated, Category="NetworkPowerSwitch")
	bool bConnected;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"), Category="NetworkPowerSwitch")
	UFGPowerConnectionComponent* PowerConnection1 = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "NetworkPowerSwitch")
	UFGPowerConnectionComponent* PowerConnection2 = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "NetworkPowerSwitch")
	UFGPowerInfoComponent* PowerInfo1 = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "NetworkPowerSwitch")
	UFGPowerInfoComponent* PowerInfo2 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NetworkPowerSwitch")
	UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	AFINNetworkPowerSwitch();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float dt) override;
	// End AFGBuildable

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFINNetworkCustomType
	virtual FString GetCustomTypeName_Implementation() const override { return TEXT("PowerSwitch"); }
	// End IFINNetworkCustomType

	/**
	 * Changes the connection state of the power switch.
	 * true if it should transfer energy, false if not.
	 * Might queue a trigger of OnConnectedChanged
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Components")
	void SetConnected(bool bNewConnected);

	/**
	 * Notifies when the connection state has changed.
	 * Gets only triggerd in actor tick.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, NetMulticast, Category="Network|Components")
	void OnConnectedChanged();

	UFUNCTION(BlueprintCallable, Category="Network|Component")
    void netFunc_setConnected(bool newConnected);

	UFUNCTION(BlueprintCallable, Category="Network|Component")
    bool netFunc_isConnected();
};